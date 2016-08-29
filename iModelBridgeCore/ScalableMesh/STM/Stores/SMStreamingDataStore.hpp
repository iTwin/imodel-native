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
#include <CloudDataSource\DataSourceBuffered.h>

#include <ImagePP\all\h\HCDCodecZlib.h>
#include <ImagePP\all\h\HFCAccessMode.h>
#include "ScalableMesh/ScalableMeshLib.h"

USING_NAMESPACE_IMAGEPP



template <class EXTENT> SMStreamingStore<EXTENT>::SMStreamingStore(DataSourceAccount *dataSourceAccount, bool compress, bool areNodeHeadersGrouped, bool isVirtualGrouping, WString headers_path)
    :m_pathToHeaders(headers_path),
     m_use_node_header_grouping(areNodeHeadersGrouped),
     m_use_virtual_grouping(isVirtualGrouping)
    {
    Utf8String token1 = ScalableMesh::ScalableMeshLib::GetHost().GetWsgTokenAdmin().GetToken();
    assert(!token1.empty());
    Utf8String token2 = ScalableMesh::ScalableMeshLib::GetHost().GetWsgTokenAdmin().GetToken();
    assert(!token2.empty());

    SetDataSourceAccount(dataSourceAccount);
       
    if (m_pathToHeaders.empty())
        {
        // Set default path to headers relative to root directory
        m_pathToHeaders = L"headers/";

        if (m_use_node_header_grouping && m_use_virtual_grouping)
            {
            m_NodeHeaderFetchDistributor = new SMNodeDistributor<SMNodeGroup::DistributeData>();
            SMNodeGroup::SetWorkTo(*m_NodeHeaderFetchDistributor);
            }
        }

    // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
    //                           and do this in the Cloud API...
    if (s_stream_from_disk)
        {
        // Create base directory structure to store information if not already done
        // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only
        if (0 == CreateDirectoryW(m_dataSourceAccount->getPrefixPath().c_str(), NULL))
            {
            assert(ERROR_PATH_NOT_FOUND != GetLastError());
            }        
        
        if (0 == CreateDirectoryW((WString(m_dataSourceAccount->getPrefixPath().c_str()) + m_pathToHeaders).c_str(), NULL))
            {
            assert(ERROR_PATH_NOT_FOUND != GetLastError());
            }        
        }
    }

template <class EXTENT> SMStreamingStore<EXTENT>::~SMStreamingStore()
    {
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

        auto buffer = Json::StyledWriter().write(masterHeader);
        uint64_t buffer_size = buffer.size();

        DataSourceURL    dataSourceURL(L"MasterHeader.sscm");

        DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource();
        assert(dataSource != nullptr);

        if (dataSource->open(dataSourceURL, DataSourceMode_Write).isFailed())
            {
            assert(false); // could not open master header data source!
            return false;
            }

        if (dataSource->write((const DataSource::Buffer*)buffer.c_str(), buffer_size).isFailed())
            {
            assert(false); // error writing to master header data source!
            return false;
            }

        if (dataSource->close().isFailed())
            {
            assert(false); // error closing master header data source!
            return false;
            }
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

            swprintf(buffer, L"MasterHeaderWith%sGroups.bin", (m_use_virtual_grouping ? L"Virtual" : L""));

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

                short groupMode = m_use_virtual_grouping;
                memcpy(&groupMode, reinterpret_cast<char *>(dest.get()) + position, sizeof(groupMode));
                assert((groupMode == SMNodeGroup::VIRTUAL) == s_is_virtual_grouping); // Trying to load streaming master header with incoherent grouping strategies
                position += sizeof(groupMode);


                // Parse rest of file -- group information
                while (position < headerSize)
                    {
                    size_t group_id;
                    memcpy(&group_id, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_id));
                    position += sizeof(group_id);

                    uint64_t group_totalSizeOfHeaders(0);
                    if (groupMode == SMNodeGroup::VIRTUAL)
                        {
                        memcpy(&group_totalSizeOfHeaders, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_totalSizeOfHeaders));
                        position += sizeof(group_totalSizeOfHeaders);
                        }

                    size_t group_numNodes;
                    memcpy(&group_numNodes, reinterpret_cast<char *>(dest.get()) + position, sizeof(size_t));
                    position += sizeof(size_t);

                    auto group = HFCPtr<SMNodeGroup>(new SMNodeGroup(this->GetDataSourceAccount(), 
                                                                     group_id, 
                                                                     SMNodeGroup::Mode(groupMode),
                                                                     group_numNodes, 
                                                                     group_totalSizeOfHeaders));
                    // NEEDS_WORK_SM : group datasource doesn't need to depend on type of grouping
                    group->SetDataSource(groupMode == SMNodeGroup::VIRTUAL ? m_pathToHeaders : m_pathToHeaders + L"g\\g_");
                    group->SetDistributor(*m_NodeHeaderFetchDistributor);
                    m_nodeHeaderGroups.push_back(group);

                    vector<uint64_t> nodeIds(group_numNodes);
                    memcpy(nodeIds.data(), reinterpret_cast<char *>(dest.get()) + position, group_numNodes*sizeof(uint64_t));
                    position += group_numNodes*sizeof(uint64_t);

                    group->GetHeader()->resize(group_numNodes);
                    transform(begin(nodeIds), end(nodeIds), begin(*group->GetHeader()), [](const uint64_t& nodeId)
                        {
                        return SMNodeHeader{ nodeId, uint32_t(-1), 0 };
                        });
                    }
                }
            }
        else
            {
            Json::Reader    reader;
            Json::Value     masterHeader;

            DataSourceURL dataSourceURL(L"MasterHeader.sscm");
            
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
            // NEW_SSTORE_RB Temporary fix until terrain is correctly implemented for streaming
            indexHeader->m_isTerrain = false/*masterHeader["isTerrain"].asBool()*/; 

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
    auto nbDataSizes = pi_pHeader->m_blockSizes.size();
    memcpy(po_pBinaryData.get() + po_pDataSize, &nbDataSizes, sizeof(nbDataSizes));
    po_pDataSize += sizeof(nbDataSizes);

    memcpy(po_pBinaryData.get() + po_pDataSize, pi_pHeader->m_blockSizes.data(), nbDataSizes * sizeof(SMIndexNodeHeader<EXTENT>::BlockSize));
    po_pDataSize += (uint32_t)(nbDataSizes * sizeof(SMIndexNodeHeader<EXTENT>::BlockSize));
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

    DataSourceURL    dataSourceURL(buffer);

    bool created = false;
    DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource(&created);
    if (dataSource == nullptr)
        {
        assert(false); // problem creating new datasource
        return 0;
        }
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //if (!created) std::cout << "[" << std::this_thread::get_id() << "] A datasource is being reused by thread" << std::endl;
    //else std::cout << "[" << std::this_thread::get_id() << "] New thread DataSource created" << std::endl;
    //}

    if (dataSource->open(dataSourceURL, DataSourceMode_Write).isFailed())
        {
        assert(false); // problem opening a datasource
        return 0;
        }

    if (dataSource->write(headerData.get(), headerSize).isFailed())
        {
        assert(false); // problem writing a datasource
        return 0;
        }

    if (dataSource->close().isFailed())
        {
        assert(false); // problem closing a datasource
        return 0;
        }
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //std::cout << "[" << std::this_thread::get_id() << "] Thread DataSource finished" << std::endl;
    //}

    return 1;
    }
    
template <class EXTENT> size_t SMStreamingStore<EXTENT>::LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID)            
    {
    if (s_stream_from_grouped_store)
        {
        auto group = this->GetGroup(blockID);
        auto node_header = group->GetNodeHeader(blockID.m_integerID);
        ReadNodeHeaderFromBinary(header, group->GetRawHeaders(node_header.offset), node_header.size);
        header->m_id = blockID;
        //group->removeNodeData(blockID.m_integerID);
        }
    else {
        //auto nodeHeader = this->GetNodeHeaderJSON(blockID);
        //ReadNodeHeaderFromJSON(header, nodeHeader);
        uint64_t headerSize = 0;
        std::unique_ptr<Byte> headerData = nullptr;
        this->GetNodeHeaderBinary(blockID, headerData, headerSize);
        if (!headerData && headerSize == 0) return 0;
        ReadNodeHeaderFromBinary(header, headerData.get(), headerSize);
        }
    return 1;
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::SetProjectFilesPath(BeFileName& projectFilesPath)
    {
    assert(!"Should be handled by local file");
    return false;
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
    if (group == nullptr) return group;
    if (!group->IsLoaded())
        {
        group->Load(blockID.m_integerID);
        }
#ifdef DEBUG_STREAMING_DATA_STORE
    else {
        static std::atomic<uint64_t> s_NbAlreadyLoadedNodes = 0;
        s_NbAlreadyLoadedNodes += 1;
        {
        std::lock_guard<mutex> clk(s_consoleMutex);
        std::cout << "[" << blockID.m_integerID << ", " << group->GetID() << "] already loaded in temporary streaming data cache!" << std::endl;
        }
        }
#endif
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
    assert(header->m_meshComponents == nullptr);
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
    if (dataIndex == maxCountData)
        {
        return;
        }
    uint64_t nbDataSizes;
    memcpy(&nbDataSizes, headerData + dataIndex, sizeof(nbDataSizes));
    dataIndex += sizeof(nbDataSizes);

    header->m_blockSizes.resize(nbDataSizes);
    memcpy(header->m_blockSizes.data(), headerData + dataIndex, nbDataSizes* sizeof(SMIndexNodeHeader<EXTENT>::BlockSize));
    dataIndex += nbDataSizes * sizeof(SMIndexNodeHeader<EXTENT>::BlockSize);

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


template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {   
    assert(!"Should not be called");
    return false;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {        
    assert(!"Should not be called");
    return false;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {
    //NEW_SSTORE_RB : Need to be implement
    assert(dataType != SMStoreDataType::Skirt && dataType != SMStoreDataType::ClipDefinition);
    auto nodeGroup = this->GetGroup(nodeHeader->m_id);
    dataStore = new SMStreamingNodeDataStore<DPoint3d, EXTENT>(m_dataSourceAccount, dataType, nodeHeader, nodeGroup);

    return true;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                
    assert(dataType == SMStoreDataType::TriPtIndices || dataType == SMStoreDataType::TriUvIndices);
        
    dataStore = new SMStreamingNodeDataStore<int32_t, EXTENT>(m_dataSourceAccount, dataType, nodeHeader);
                    
    return true;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {    
    dataStore = new StreamingNodeTextureStore<Byte, EXTENT>(m_dataSourceAccount, nodeHeader);
    
    return true;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {    
    dataStore = new SMStreamingNodeDataStore<DPoint2d, EXTENT>(m_dataSourceAccount, SMStoreDataType::UvCoords, nodeHeader);

    return true;    
    }


//Multi-items loading store

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {    
    //dataStore = new SMStreamingNodeDataStore<int32_t, EXTENT>(m_dataSourceAccount, SMStoreDataType::TriPtIndices, nodeHeader);
    assert(!"Not supported yet");

    return false;    
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


//------------------SMStreamingNodeDataStore--------------------------------------------
template <class DATATYPE, class EXTENT> SMStreamingNodeDataStore<DATATYPE, EXTENT>::SMStreamingNodeDataStore(DataSourceAccount* dataSourceAccount, SMStoreDataType type, SMIndexNodeHeader<EXTENT>* nodeHeader, HFCPtr<SMNodeGroup> nodeGroup, bool compress = true)
    :m_dataSourceAccount(dataSourceAccount),     
     m_nodeGroup(nodeGroup)
    {       
    m_nodeHeader = nodeHeader;
    m_dataType = type;

    switch (type)
        {
        case SMStoreDataType::Points: 
            m_pathToNodeData = L"points/";
            break;
        case SMStoreDataType::TriPtIndices:
            m_pathToNodeData = L"indices/";
            break;
        case SMStoreDataType::UvCoords:
            m_pathToNodeData = L"uvs/";
            break;
        case SMStoreDataType::TriUvIndices:
            m_pathToNodeData = L"uvindices/";
            break;
        default:
            assert(!"Unkown data type for streaming");
        }


    // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
    if (s_stream_from_disk)
        {
        // Create base directory structure to store information if not already done
        // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only        
        if (0 == CreateDirectoryW((WString(m_dataSourceAccount->getPrefixPath().c_str()) + m_pathToNodeData).c_str(), NULL))
            {
            assert(ERROR_PATH_NOT_FOUND != GetLastError());
            }        
        }
    else
        {
        // stream from azure
        }
    }

template <class DATATYPE, class EXTENT> SMStreamingNodeDataStore<DATATYPE, EXTENT>::~SMStreamingNodeDataStore()
    {            
    for (auto it = m_pointCache.begin(); it != m_pointCache.end(); ++it) it->second.clear();
    m_pointCache.clear();
    }

template <class DATATYPE, class EXTENT> HPMBlockID SMStreamingNodeDataStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    HPRECONDITION(blockID.IsValid());

    if (NULL != DataTypeArray && countData > 0)
        {
        DataSourceStatus writeStatus;
        wchar_t buffer[10000];
        swprintf(buffer, L"%sp_%llu.bin", m_pathToNodeData.c_str(), blockID.m_integerID);
        DataSourceURL    dataSourceURL(buffer);

        bool created = false;
        DataSourceBuffered *dataSource = dynamic_cast<DataSourceBuffered*>(m_dataSourceAccount->getOrCreateThreadDataSource(&created));
        //{
        //std::lock_guard<mutex> clk(s_consoleMutex);
        //if (!created) std::cout << "[" << std::this_thread::get_id() << "] A datasource is being reused by thread" << std::endl;
        //else std::cout<<"[" << std::this_thread::get_id() << "] New thread DataSource created" << std::endl;
        //}
        //DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource();
        assert(dataSource != nullptr); // problem creating a new DataSource

        dataSource->setSegmentSize(1024 * 32);

        // Time I/O operation timeouts for threading
        dataSource->setTimeout(DataSource::Timeout(10000));

        writeStatus = dataSource->open(dataSourceURL, DataSourceMode_Write_Segmented);
        assert(writeStatus.isOK()); // problem opening a DataSource

        HCDPacket uncompressedPacket, compressedPacket;
        size_t bufferSize = countData * sizeof(DATATYPE);
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

        m_nodeHeader->m_blockSizes.push_back(SMIndexNodeHeader<EXTENT>::BlockSize{ compressedPacket.GetDataSize() + sizeof(uint32_t), (short)m_dataType });

        memcpy(data + sizeof(uint32_t), compressedPacket.GetBufferAddress(), compressedPacket.GetDataSize());

        writeStatus = dataSource->write(data, (uint32_t)compressedPacket.GetDataSize() + sizeof(uint32_t));
        assert(writeStatus.isOK()); // problem writing a DataSource

        writeStatus = dataSource->close();
        assert(writeStatus.isOK()); // problem closing a DataSource

        //{
        //std::lock_guard<mutex> clk(s_consoleMutex);
        //std::cout << "[" << std::this_thread::get_id() << "] Thread DataSource finished" << std::endl;
        //}
        delete[] data;
        delete[] dataArrayTmp;
        }

    return blockID;   
    }

template <class DATATYPE, class EXTENT> size_t SMStreamingNodeDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
    {
    if (m_dataType == SMStoreDataType::Points)
        return m_nodeHeader->m_nodeCount;
    else
    if (m_dataType == SMStoreDataType::TriPtIndices)
        return m_nodeHeader->m_nbFaceIndexes;    
    else       
    if (IsValidID(blockID))
        return this->GetBlock(blockID).size() / sizeof(DATATYPE);

    return 0;   
    }

template <class DATATYPE, class EXTENT> size_t SMStreamingNodeDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const
    {
    assert(!"Invalid call");
    return 0;
    }

template <class DATATYPE, class EXTENT> void SMStreamingNodeDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) 
    {
    switch (m_dataType)
        {
        case SMStoreDataType::Points : 
            assert((((int64_t)m_nodeHeader->m_nodeCount) + countDelta) >= 0);
            m_nodeHeader->m_nodeCount += countDelta;                
            break;
         case SMStoreDataType::TriPtIndices : 
            assert((((int64_t)m_nodeHeader->m_nbFaceIndexes) + countDelta) >= 0);
            m_nodeHeader->m_nbFaceIndexes += countDelta;                
            break;
        case SMStoreDataType::UvCoords :
        case SMStoreDataType::TriUvIndices :            
            //MST_TS
            break;
        default : 
            assert(!"Unsupported type");
            break;
        }        
    }

template <class DATATYPE, class EXTENT> void SMStreamingNodeDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) 
    {
    assert(!"Invalid call");
    }

template <class DATATYPE, class EXTENT> size_t SMStreamingNodeDataStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    auto& block = this->GetBlock(blockID);
    auto blockSize = block.size();
    assert(block.size() <= maxCountData * sizeof(DATATYPE));
    memmove(DataTypeArray, block.data(), block.size());

    m_pointCacheLock.lock();
    block.UnLoad();
    m_pointCache.erase(block.GetID());
    m_pointCacheLock.unlock();

    return blockSize;
    }

template <class DATATYPE, class EXTENT> bool SMStreamingNodeDataStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return true;
    }

template <class DATATYPE, class EXTENT> StreamingDataBlock& SMStreamingNodeDataStore<DATATYPE, EXTENT>::GetBlock(HPMBlockID blockID) const
    {
    // std::map [] operator is not thread safe while inserting new elements
    m_pointCacheLock.lock();
    StreamingDataBlock& block = m_pointCache[blockID.m_integerID];
    m_pointCacheLock.unlock();
    assert((block.GetID() != uint64_t(-1) ? block.GetID() == blockID.m_integerID : true));
    if (!block.IsLoaded())
        {
        auto blockSize = m_nodeHeader->GetBlockSize((short)m_dataType);
        //assert(blockSize != uint64_t(-1));
        block.SetID(blockID.m_integerID);
        block.SetDataSource(m_pathToNodeData);        
        block.Load(m_dataSourceAccount, blockSize);
        }
    assert(block.GetID() == blockID.m_integerID);
    assert(block.IsLoaded() && !block.empty());
    return block;
    }



bool StreamingDataBlock::IsLoading() 
    { 
    return m_pIsLoading; 
    }

bool StreamingDataBlock::IsLoaded() 
    { 
    return m_pIsLoaded; 
    }

void StreamingDataBlock::LockAndWait()
    {
    unique_lock<mutex> lock(m_pPointBlockMutex);
    m_pPointBlockCV.wait(lock, [this]() { return m_pIsLoaded; });
    }

void StreamingDataBlock::SetLoading() 
    { 
    m_pIsLoading = true; 
    }

DataSource* StreamingDataBlock::initializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize)
    {
    if (dataSourceAccount == nullptr)
        return nullptr;
                                                        // Get the thread's DataSource or create a new one
    DataSource *dataSource = dataSourceAccount->getOrCreateThreadDataSource();
    if (dataSource == nullptr)
        return nullptr;
                                                        // Make sure caching is enabled for this DataSource
    dataSource->setCachingEnabled(s_stream_enable_caching);

    dest.reset(new unsigned char[destSize]);
                                                        // Return the DataSource
    return dataSource;
    }
    
void StreamingDataBlock::Load(DataSourceAccount *dataSourceAccount, uint64_t dataSize)
    {
    if (!IsLoaded())
        {
        if (IsLoading())
            {
                LockAndWait();
            }
        else
            {
                wchar_t buffer[10000];
                swprintf(buffer, L"%sp_%llu.bin", m_pDataSource.c_str(), m_pID);
            
                std::unique_ptr<DataSource::Buffer[]>        dest;
                DataSource                                *  dataSource;
                DataSource::DataSize                         readSize;

                DataSourceURL    dataSourceURL(buffer);

                DataSourceBuffer::BufferSize    destSize = 5 * 1024 * 1024;

                dataSource = initializeDataSource(dataSourceAccount, dest, destSize);
                assert(dest != nullptr);
                if (dataSource == nullptr)
                    return;

                if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                    return;

                if (dataSize == uint64_t(-1)) dataSize = 0;
                if (dataSource->read(dest.get(), destSize, readSize, dataSize).isFailed())
                    return;

                if (dataSource->close().isFailed())
                    return;

//                dataSourceAccount->destroyDataSource(dataSource);

                if (readSize > 0)
                {
                    m_pIsLoaded = true;

                    uint32_t uncompressedSize = *reinterpret_cast<uint32_t *>(dest.get());
                    //std::wcout << "node id:" << m_pID << "  source: " << m_pDataSource << "  size(bytes) : " << dataSize << std::endl;
                    uint32_t sizeData = (uint32_t)readSize - sizeof(uint32_t);
                    DecompressPoints(dest.get() + sizeof(uint32_t), sizeData, uncompressedSize);
                }
            }
        }
    }
    
void StreamingDataBlock::UnLoad()
    {
    m_pIsLoaded = false;
    m_pIsLoading = false;
    this->clear();
    }
    
void StreamingDataBlock::SetLoaded()
    {
    m_pIsLoaded = true;
    m_pIsLoading = false;
    m_pPointBlockCV.notify_all();
    }

void StreamingDataBlock::SetID(const uint64_t& pi_ID)
    {
    m_pID = pi_ID;
    }

uint64_t StreamingDataBlock::GetID() 
    { 
    return m_pID; 
    }

void StreamingDataBlock::SetDataSource(const WString& pi_DataSource)
    {
    m_pDataSource = pi_DataSource;
    }

void StreamingDataBlock::DecompressPoints(uint8_t* pi_CompressedData, uint32_t pi_CompressedDataSize, uint32_t pi_UncompressedDataSize)
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

// NEEDS_WORK_SM_STREAMING: Move this to the CloudDataSource?
//size_t StreamingDataBlock::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
//    {
//    size_t realsize = size * nmemb;
//    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
//
//    assert(mem->memory->capacity() >= mem->memory->size() + realsize);
//
//    //    mem->memory->assign((Byte*)contents, (Byte*)contents + realsize);
//    mem->memory->insert(mem->memory->end(), (Byte*)contents, (Byte*)contents + realsize);
//
//    return realsize;
//    }
//
//// NEEDS_WORK_SM_STREAMING: Move this to the CloudDataSource?
//void StreamingDataBlock::LoadFromFileSystem(const WString& m_pFilename)
//    {
//    CURL *curl_handle;
//    bool retCode = true;
//    struct MemoryStruct chunk;
//    const int maxCountData = 100000;
//    Utf8String blockUrl = "http://realitydatastreaming.azurewebsites.net/Mesh/c1sub_scalablemesh/";
//    Utf8String name;
//    BeStringUtilities::WCharToUtf8(name, m_pFilename.c_str());
//    blockUrl += name;
//
//    chunk.memory = this;
//    chunk.memory->reserve(maxCountData);
//    chunk.size = 0;
//    curl_global_init(CURL_GLOBAL_ALL);
//    curl_handle = curl_easy_init();
//
//    // NEEDS_WORK_SM_STREAMING: Remove this when streaming works reasonably well
//    //std::lock_guard<std::mutex> lock(fileMutex);
//    //BeFile file;
//    //if (BeFileStatus::Success == OPEN_FILE(file, L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\FitView.node", BeFileAccess::ReadWrite) ||
//    //    BeFileStatus::Success == file.Create(L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\FitView.node"))
//    //    {
//    //    file.SetPointer(0, BeFileSeekOrigin::End);
//    //    auto node_location = L"http://realitydatastreaming.azurewebsites.net/Mesh/c1sub_scalablemesh/" + block_name + L"\n";
//    //    Utf8String utf8_node_location;
//    //    BeStringUtilities::WCharToUtf8(utf8_node_location, node_location.c_str());
//    //    file.Write(NULL, utf8_node_location.c_str(), (uint32_t)utf8_node_location.length());
//    //    }
//    //else
//    //    {
//    //    assert(!"Problem creating nodes file");
//    //    }
//    //
//    //file.Close();
//
//    curl_easy_setopt(curl_handle, CURLOPT_URL, blockUrl.c_str());
//    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
//    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
//    //    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
//
//    /* get it! */
//    CURLcode res = curl_easy_perform(curl_handle);
//    /* check for errors */
//    if (CURLE_OK != res)
//        {
//        retCode = false;
//        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
//        assert(false);
//        }
//
//    curl_easy_cleanup(curl_handle);
//    curl_global_cleanup();
//
//    assert(!chunk.memory->empty() && chunk.memory->size() <= 1000000);
//
//    uint32_t uncompressedSize = reinterpret_cast<uint32_t&>((*chunk.memory)[0]);
//    uint32_t sizeData = (uint32_t)chunk.memory->size() - sizeof(uint32_t);
//
//    this->DecompressPoints(&(*chunk.memory)[0] + sizeof(uint32_t), sizeData, uncompressedSize);
//    }


template <class DATATYPE, class EXTENT> Texture& StreamingNodeTextureStore<DATATYPE, EXTENT>::GetTexture(HPMBlockID blockID) const
    {
    // std::map [] operator is not thread safe while inserting new elements
    m_textureCacheLock.lock();
    Texture& texture = m_textureCache[blockID.m_integerID];
    m_textureCacheLock.unlock();
    assert((texture.GetID() != uint64_t(-1) ? texture.GetID() == blockID.m_integerID : true));
    if (!texture.IsLoaded())
        {
        if (texture.IsLoading())
            {
            texture.LockAndWait();
            }
        else
            {
            auto blockSize = m_nodeHeader->GetBlockSize(5);
            texture.SetDataSource(this->GetDataSourceAccount(), m_path);
            texture.SetID(blockID.m_integerID);
            texture.Load(blockSize);
            }
        }
    assert(texture.IsLoaded() && !texture.empty());
    return texture;
    }


template <class DATATYPE, class EXTENT> StreamingNodeTextureStore<DATATYPE, EXTENT>::StreamingNodeTextureStore(DataSourceAccount *dataSourceAccount, SMIndexNodeHeader<EXTENT>* nodeHeader)
    : m_path(L"textures/"),
      m_nodeHeader(nodeHeader)
    {
    this->SetDataSourceAccount(dataSourceAccount);

    // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
    if (s_stream_from_disk)
        {
        // Create base directory structure to store information if not already done
        // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only
        if (0 == CreateDirectoryW((WString(m_dataSourceAccount->getPrefixPath().c_str()) + m_path).c_str(), NULL))
            {
            assert(ERROR_PATH_NOT_FOUND != GetLastError());
            }
        }
    }

template <class DATATYPE, class EXTENT> StreamingNodeTextureStore<DATATYPE, EXTENT>::~StreamingNodeTextureStore()
    {
    for (auto it = m_textureCache.begin(); it != m_textureCache.end(); ++it) it->second.clear();
    m_textureCache.clear();
    }

template <class DATATYPE, class EXTENT> bool StreamingNodeTextureStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return false;
    }
                        
template <class DATATYPE, class EXTENT> HPMBlockID StreamingNodeTextureStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    assert(blockID.IsValid());

    // The data block starts with 12 bytes of metadata (texture header), followed by pixel data
    Texture texture(((int*)DataTypeArray)[0], ((int*)DataTypeArray)[1], ((int*)DataTypeArray)[2]);
    texture.SetDataSource(m_dataSourceAccount, m_path);
    texture.SavePixelDataToDisk(DataTypeArray + 3 * sizeof(int), countData - 3 * sizeof(int), blockID);

    return blockID;
    }

template <class DATATYPE, class EXTENT> HPMBlockID StreamingNodeTextureStore<DATATYPE, EXTENT>::StoreCompressedBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    assert(blockID.IsValid());

    DataSourceStatus writeStatus;
    wchar_t buffer[10000];
    swprintf(buffer, L"%st_%llu.bin", m_path.c_str(), blockID.m_integerID);
    DataSourceURL    dataSourceURL(buffer);

    bool created = false;
    DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource(&created);
    assert(dataSource != nullptr);
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //if (!created) std::cout << "[" << std::this_thread::get_id() << "] A datasource is being reused by thread" << std::endl;
    //else std::cout << "[" << std::this_thread::get_id() << "] New thread DataSource created" << std::endl;
    //}

    writeStatus = dataSource->open(dataSourceURL, DataSourceMode_Write_Segmented);
    assert(writeStatus.isOK());

    writeStatus = dataSource->write(DataTypeArray, (uint32_t)countData);
    assert(writeStatus.isOK());

    writeStatus = dataSource->close();
    assert(writeStatus.isOK());
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //std::cout << "[" << std::this_thread::get_id() << "] Thread DataSource finished" << std::endl;
    //}

    return HPMBlockID(blockID.m_integerID);
    }

template <class DATATYPE, class EXTENT> size_t StreamingNodeTextureStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
    {
    return this->GetTexture(blockID).size() + 3 * sizeof(int);
    }

template <class DATATYPE, class EXTENT> size_t StreamingNodeTextureStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const
    {
    assert(!"Not supported");
    return 0;
    }

template <class DATATYPE, class EXTENT> void StreamingNodeTextureStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) 
    {
    }

template <class DATATYPE, class EXTENT> void StreamingNodeTextureStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) 
    {
    assert(!"Not supported");    
    }

template <class DATATYPE, class EXTENT> size_t StreamingNodeTextureStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    auto& texture = this->GetTexture(blockID);
    auto textureSize = texture.size();
    assert(textureSize + 3 * sizeof(int) == maxCountData);
    ((int*)DataTypeArray)[0] = (int)texture.GetWidth();
    ((int*)DataTypeArray)[1] = (int)texture.GetHeight();
    ((int*)DataTypeArray)[2] = (int)texture.GetNbChannels();
    assert(maxCountData >= texture.size());
    memmove(DataTypeArray + 3 * sizeof(int), texture.data(), std::min(texture.size(), maxCountData));
    m_textureCacheLock.lock();
    texture.Unload();
    m_textureCache.erase(texture.GetID());
    m_textureCacheLock.unlock();
    return std::min(textureSize + 3 * sizeof(int), maxCountData);
    }

template <class DATATYPE, class EXTENT> void StreamingNodeTextureStore<DATATYPE, EXTENT>::SetDataSourceAccount    (DataSourceAccount *dataSourceAccount)  
    {
    m_dataSourceAccount = dataSourceAccount;
    }

template <class DATATYPE, class EXTENT> DataSourceAccount* StreamingNodeTextureStore<DATATYPE, EXTENT>::GetDataSourceAccount(void) const                            
    {
    return m_dataSourceAccount;
    }