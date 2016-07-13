//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMStreamingDataStore.hpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include "SMStreamingDataStore.h"
#include "../Threading\LightThreadPool.h"
#include <curl/curl.h>
#include <condition_variable>
#include <CloudDataSource/DataSourceAccount.h>
#ifdef VANCOUVER_API
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#endif

extern bool s_stream_from_disk;
extern bool s_stream_from_grouped_store;
extern bool s_is_virtual_grouping;



template <class EXTENT> SMStreamingStore<EXTENT>::SMStreamingStore(DataSourceAccount *dataSourceAccount, const WString& path, bool compress, bool areNodeHeadersGrouped, WString headers_path)
    :m_rootDirectory(path),     
     m_pathToHeaders(headers_path),
     m_use_node_header_grouping(areNodeHeadersGrouped)
    {
    SetDataSourceAccount(dataSourceAccount);            
       
    if (m_pathToHeaders.empty())
        {
        // Set default path to headers relative to root directory
        m_pathToHeaders = m_rootDirectory + L"headers/";

        if (m_use_node_header_grouping && s_is_virtual_grouping)
            {
            m_NodeHeaderFetchDistributor = new SMNodeDistributor<SMNodeGroup::DistributeData>();
            SMNodeGroup::SetWorkTo(*m_NodeHeaderFetchDistributor);
            }
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
        
        if (0 == CreateDirectoryW(m_pathToHeaders.c_str(), NULL))
            {
            assert(ERROR_PATH_NOT_FOUND != GetLastError());
            }        
        }
    }

template <class EXTENT> SMStreamingStore<EXTENT>::~SMStreamingStore()
    {
    if (m_NodeHeaderFetchDistributor)
        {
        m_NodeHeaderFetchDistributor->CancelAll();
        m_NodeHeaderFetchDistributor = nullptr;
        }
    }

template <class EXTENT> uint64_t SMStreamingStore<EXTENT>::GetNextID() const
    {
    assert(!"Not implemented yet");
    return 0; 
    }
            
template <class EXTENT> void SMStreamingStore<EXTENT>::Close()
    {
    }
            
template <class EXTENT> bool SMStreamingStore<EXTENT>::StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize)
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
    
template <class EXTENT> size_t SMStreamingStore<EXTENT>::LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize)
    {
    std::unique_ptr<DataSource::Buffer[]>            dest;
    DataSource                                *      dataSource;
    DataSource::DataSize                             readSize;
    DataSourceBuffer::BufferSize                     destSize = 20 * 1024 * 1024;

    if (indexHeader != NULL)
        {
        if (m_use_node_header_grouping || s_stream_from_grouped_store)
            {
            wchar_t buffer[10000];

            swprintf(buffer, L"%sMasterHeaderWithGroups.bin", m_rootDirectory.c_str());

            DataSourceURL    dataSourceURL(buffer);
                                

            if (m_nodeHeaderGroups.empty())
                {
                dataSource = this->InitializeDataSource(dest, destSize);
                if (dataSource == nullptr)
                    return 0;

                if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                    return 0;

                if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
                    return 0;

                assert(destSize >= readSize); 

                dataSource->close();

                headerSize = readSize;

                uint64_t position = 0;

                uint32_t sizeOfOldMasterHeaderPart;
                memcpy(&sizeOfOldMasterHeaderPart, dest.get() + position, sizeof(sizeOfOldMasterHeaderPart));
                position += sizeof(sizeOfOldMasterHeaderPart);
                assert(sizeOfOldMasterHeaderPart == sizeof(SQLiteIndexHeader));

                SQLiteIndexHeader oldMasterHeader;
                memcpy(&oldMasterHeader, dest.get() + position, sizeof(SQLiteIndexHeader));
                position += sizeof(SQLiteIndexHeader);
                indexHeader->m_SplitTreshold = oldMasterHeader.m_SplitTreshold;
                indexHeader->m_balanced = oldMasterHeader.m_balanced;
                indexHeader->m_depth = oldMasterHeader.m_depth;
                indexHeader->m_isTerrain = oldMasterHeader.m_isTerrain;
                indexHeader->m_singleFile = oldMasterHeader.m_singleFile;
                assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

                auto rootNodeBlockID = oldMasterHeader.m_rootNodeBlockID;
                indexHeader->m_rootNodeBlockID = rootNodeBlockID != ISMStore::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

                memcpy(&s_is_virtual_grouping, reinterpret_cast<char *>(dest.get()) + position, sizeof(s_is_virtual_grouping));
                position += sizeof(s_is_virtual_grouping);


                // Parse rest of file -- group information
                while (position < headerSize)
                    {
                    size_t group_id;
                    memcpy(&group_id, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_id));
                    position += sizeof(group_id);

                    uint64_t group_totalSizeOfHeaders(0);
                    if (s_is_virtual_grouping)
                        {
                        memcpy(&group_totalSizeOfHeaders, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_totalSizeOfHeaders));
                        position += sizeof(group_totalSizeOfHeaders);
                        }

                    size_t group_numNodes;
                    memcpy(&group_numNodes, reinterpret_cast<char *>(dest.get()) + position, sizeof(size_t));
                    position += sizeof(size_t);
                    //assert(group_size <= s_max_number_nodes_in_group);

                    auto group = HFCPtr<SMNodeGroup>(new SMNodeGroup(this->GetDataSourceAccount(), group_id, group_numNodes, group_totalSizeOfHeaders));
                    // NEEDS_WORK_SM : group datasource doesn't need to depend on type of grouping
                    group->SetDataSource(s_is_virtual_grouping ? m_pathToHeaders : m_pathToHeaders + L"g_");
                    group->SetDistributor(*m_NodeHeaderFetchDistributor);
                    m_nodeHeaderGroups.push_back(group);

                    vector<uint64_t> nodeIds(group_numNodes);
                    memcpy(nodeIds.data(), reinterpret_cast<char *>(dest.get()) + position, group_numNodes*sizeof(uint64_t));
                    position += group_numNodes*sizeof(uint64_t);

                    group->GetHeader()->resize(group_numNodes);
                    transform(begin(nodeIds), end(nodeIds), begin(*group->GetHeader()), [](const uint64_t& nodeId)
                        {
                        return SMNodeHeader    { nodeId, 0, 0     };
                        });
                    }
                }
            }
        else
            {
            Json::Reader    reader;
            Json::Value     masterHeader;

            DataSourceURL dataSourceURL(m_rootDirectory.data());
            
            dataSourceURL.append(L"MasterHeader.sscm");

            dataSource = this->InitializeDataSource(dest, destSize);
            if (dataSource == nullptr)
                return 0;

            if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                return 0;

            if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
                return 0;

            dataSource->close();

            headerSize = readSize;

            reader.parse(reinterpret_cast<char *>(dest.get()), reinterpret_cast<char *>(&(dest.get()[readSize])), masterHeader);

            indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
            indexHeader->m_balanced = masterHeader["balanced"].asBool();
            indexHeader->m_depth = masterHeader["depth"].asUInt();
            indexHeader->m_isTerrain = masterHeader["isTerrain"].asBool();

            auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
            indexHeader->m_rootNodeBlockID = rootNodeBlockID != ISMStore::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();
/* Needed?
            if (masterHeader.isMember("singleFile"))
                {
                indexHeader->m_singleFile = masterHeader["singleFile"].asBool();
                HASSERT(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem
                }
*/
            }

        return headerSize;
        }

    return 0;
    }


template <class EXTENT> void SMStreamingStore<EXTENT>::SerializeHeaderToBinary(const SMIndexNodeHeader<EXTENT>* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize) const
    {
    assert(po_pBinaryData == nullptr && po_pDataSize == 0);

    po_pBinaryData.reset(new Byte[3000]);

    const auto filtered = pi_pHeader->m_filtered;
    memcpy(po_pBinaryData.get() + po_pDataSize, &filtered, sizeof(filtered));
    po_pDataSize += sizeof(filtered);
    const auto parentBlockID = pi_pHeader->m_parentNodeID.IsValid() ? ConvertBlockID(pi_pHeader->m_parentNodeID) : ISMStore::GetNullNodeID();
    memcpy(po_pBinaryData.get() + po_pDataSize, &parentBlockID, sizeof(parentBlockID));
    po_pDataSize += sizeof(parentBlockID);
    const auto subNodeNoSplitID = pi_pHeader->m_SubNodeNoSplitID.IsValid() ? ConvertBlockID(pi_pHeader->m_SubNodeNoSplitID) : ISMStore::GetNullNodeID();
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
    const auto graphID = pi_pHeader->m_graphID.IsValid() ? ConvertBlockID(pi_pHeader->m_graphID) : ISMStore::GetNullNodeID();
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
    const auto idx = pi_pHeader->m_ptsIndiceID[0].IsValid() ? ConvertBlockID(pi_pHeader->m_ptsIndiceID[0]) : ISMStore::GetNullNodeID();
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


template <class EXTENT> void SMStreamingStore<EXTENT>::SerializeHeaderToJSON(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& block)
    {
    block["id"] = ConvertBlockID(blockID);
    block["resolution"] = (ISMStore::NodeID)header->m_level;
    block["filtered"] = header->m_filtered;
    block["parentID"] = header->m_parentNodeID.IsValid() ? ConvertBlockID(header->m_parentNodeID) : ISMStore::GetNullNodeID();
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
            child["id"] = header->m_apSubNodeID[childInd].IsValid() ? ConvertChildID(header->m_apSubNodeID[childInd]) : ISMStore::GetNullNodeID();
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
    block["graphID"] = header->m_graphID.IsValid() ? ConvertBlockID(header->m_graphID) : ISMStore::GetNullNodeID();
    block["nbIndiceID"] = (int)header->m_ptsIndiceID.size();

    auto& indiceID = block["indiceID"];
    for (size_t i = 0; i < header->m_ptsIndiceID.size(); i++)
        {
        Json::Value& indice = (uint32_t)i >= indiceID.size() ? indiceID.append(Json::Value()) : indiceID[(uint32_t)i];
        indice = header->m_ptsIndiceID[i].IsValid() ? ConvertBlockID(header->m_ptsIndiceID[i]) : ISMStore::GetNullNodeID();
        }

    if (header->m_isTextured /*&& !header->m_textureID.empty() && IsValidID(header->m_textureID[0])*/)
        {
        block["areTextured"] = true;
        /*block["nbTextureIDs"] = (int)header->m_textureID.size();
        auto& textureIDs = block["textureIDs"];
        for (size_t i = 0; i < header->m_textureID.size(); i++)
            {
            auto convertedID = ConvertBlockID(header->m_textureID[i]);
            if (convertedID != ISMStore::GetNullNodeID())
                {
                Json::Value& textureID = (uint32_t)i >= textureIDs.size() ? textureIDs.append(Json::Value()) : textureIDs[(uint32_t)i];
                textureID = header->m_textureID[i].IsValid() ? convertedID : ISMStore::GetNullNodeID();
                }
            }*/
        block["uvID"] = header->m_uvID.IsValid() ? ConvertBlockID(header->m_uvID) : ISMStore::GetNullNodeID();

        block["nbUVIDs"] = (int)header->m_uvsIndicesID.size();
        auto& uvIndiceIDs = block["uvIndiceIDs"];
        for (size_t i = 0; i < header->m_uvsIndicesID.size(); i++)
            {
            Json::Value& uvIndice = (uint32_t)i >= uvIndiceIDs.size() ? uvIndiceIDs.append(Json::Value()) : uvIndiceIDs[(uint32_t)i];
            uvIndice = header->m_uvsIndicesID[i].IsValid() ? ConvertBlockID(header->m_uvsIndicesID[i]) : ISMStore::GetNullNodeID();
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
    block["nbClipSets"] = (uint32_t)header->m_clipSetsID.size();
    }

    
template <class EXTENT> size_t SMStreamingStore<EXTENT>::StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID)
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
    
template <class EXTENT> size_t SMStreamingStore<EXTENT>::LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID)            
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

template <class EXTENT> HFCPtr<SMNodeGroup> SMStreamingStore<EXTENT>::FindGroup(HPMBlockID blockID)
    {
    auto nodeIDToFind = ConvertBlockID(blockID);
    for (auto& group : m_nodeHeaderGroups)
        {
        if (group->ContainsNode(nodeIDToFind))
            {
            return group;
            }
        }

    return nullptr;
    }

template <class EXTENT> HFCPtr<SMNodeGroup> SMStreamingStore<EXTENT>::GetGroup(HPMBlockID blockID)
    {
    auto group = this->FindGroup(blockID);
    assert(group != nullptr);
    if (!group->IsLoaded())
        {
        group->Load(blockID.m_integerID);
        }
    return group;
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::ReadNodeHeaderFromBinary(SMIndexNodeHeader<EXTENT>* header, uint8_t* headerData, uint64_t& maxCountData) const
    {
    size_t dataIndex = 0;

    memcpy(&header->m_filtered, headerData + dataIndex, sizeof(header->m_filtered));
    dataIndex += sizeof(header->m_filtered);
    uint32_t parentNodeID;
    memcpy(&parentNodeID, headerData + dataIndex, sizeof(parentNodeID));
    header->m_parentNodeID = parentNodeID != ISMStore::GetNullNodeID() ? HPMBlockID(parentNodeID) : ISMStore::GetNullNodeID();
    dataIndex += sizeof(parentNodeID);
    uint32_t subNodeNoSplitID;
    memcpy(&subNodeNoSplitID, headerData + dataIndex, sizeof(subNodeNoSplitID));
    header->m_SubNodeNoSplitID = subNodeNoSplitID != ISMStore::GetNullNodeID() ? HPMBlockID(subNodeNoSplitID) : ISMStore::GetNullNodeID();
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
    header->m_graphID = graphID != ISMStore::GetNullNodeID() ? HPMBlockID(graphID) : ISMStore::GetNullNodeID();
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
        header->m_textureID[0] = ISMStore::GetNullNodeID();
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
        header->m_clipSetsID.push_back(clip != ISMStore::GetNullNodeID() ? HPMBlockID(clip) : ISMStore::GetNullNodeID());
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
        header->m_apSubNodeID.push_back(childID != ISMStore::GetNullNodeID() ? HPMBlockID(childID) : ISMStore::GetNullNodeID());
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
            header->m_apNeighborNodeID[neighborPosInd].push_back(neighborId != ISMStore::GetNullNodeID() ? HPMBlockID(neighborId) : ISMStore::GetNullNodeID());
            }
        }
    assert(dataIndex == maxCountData);
    }


template <class EXTENT> void SMStreamingStore<EXTENT>::GetNodeHeaderBinary(const HPMBlockID& blockID, std::unique_ptr<uint8_t>& po_pBinaryData, uint64_t& po_pDataSize)
    {
    //NEEDS_WORK_SM_STREAMING : are we loading node headers multiple times?
    std::unique_ptr<DataSource::Buffer[]>          dest;
    DataSource                                *    dataSource;
    DataSource::DataSize                           readSize;
    wchar_t                                        buffer[10000];

    swprintf(buffer, L"%sn_%llu.bin", m_pathToHeaders.c_str(), blockID.m_integerID);

    DataSourceURL    dataSourceURL(buffer);

    DataSourceBuffer::BufferSize    destSize = 5 * 1024 * 1024;

    dataSource = this->InitializeDataSource(dest, destSize);
    if (dataSource == nullptr)
        return;

    if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
        return;

    if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
        return;

    if (dataSource->close().isFailed())
        return;

    if (readSize > 0)
        {
        po_pDataSize = readSize;

        uint8_t *buffer = new uint8_t[readSize];
        if (buffer)
            {
            po_pBinaryData.reset(buffer);
            memmove(po_pBinaryData.get(), dest.get(), po_pDataSize);
            }

        assert(buffer);
        assert(readSize > 0);
        }   
    }

template <class EXTENT> RefCountedPtr<ISMNodeDataStore<DPoint3d, SMIndexNodeHeader<EXTENT>>> SMStreamingStore<EXTENT>::GetNodeDataStore(SMIndexNodeHeader<EXTENT>* nodeHeader)
    {
    RefCountedPtr<ISMNodeDataStore<DPoint3d, SMIndexNodeHeader<EXTENT>>> pointDataStore;

    return pointDataStore; 
    }

template <class EXTENT> DataSource* SMStreamingStore<EXTENT>::InitializeDataSource(std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize) const
    {
    if (this->GetDataSourceAccount() == nullptr)
        return nullptr;
                                                    // Get the thread's DataSource or create a new one
    DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource();
    if (dataSource == nullptr)
        return nullptr;
                                                    // Make sure caching is enabled for this DataSource
    dataSource->setCachingEnabled(s_stream_enable_caching);

    dest.reset(new unsigned char[destSize]);
                                                    // Return the DataSource
    return dataSource;
    }

template <class EXTENT> DataSourceAccount* SMStreamingStore<EXTENT>::GetDataSourceAccount(void) const
    {
    return m_dataSourceAccount;
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SetDataSourceAccount(DataSourceAccount *dataSourceAccount)
    {
    m_dataSourceAccount = dataSourceAccount;
    }