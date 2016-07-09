#pragma once

#include "SMSQLiteStore.h"



template<class EXTENT> SMIndexMasterHeader<EXTENT>& SMIndexMasterHeader<EXTENT>::operator=(const SQLiteIndexHeader& indexHeader)
    {
    if (indexHeader.m_rootNodeBlockID != SQLiteNodeHeader::NO_NODEID) m_rootNodeBlockID = HPMBlockID(indexHeader.m_rootNodeBlockID);
    m_balanced = indexHeader.m_balanced;
    m_depth = indexHeader.m_depth;
    m_terrainDepth = indexHeader.m_terrainDepth;
    m_HasMaxExtent = indexHeader.m_HasMaxExtent;
    m_MaxExtent = ExtentOp<EXTENT>::Create(indexHeader.m_MaxExtent.low.x, indexHeader.m_MaxExtent.low.y, indexHeader.m_MaxExtent.low.z,
                                           indexHeader.m_MaxExtent.high.x, indexHeader.m_MaxExtent.high.y, indexHeader.m_MaxExtent.high.z);
    m_numberOfSubNodesOnSplit = indexHeader.m_numberOfSubNodesOnSplit;
    m_singleFile = indexHeader.m_singleFile;
    m_SplitTreshold = indexHeader.m_SplitTreshold;
    m_textured = indexHeader.m_textured;
    m_isTerrain = indexHeader.m_isTerrain;
    return *this;
    }

template<class EXTENT> SMIndexMasterHeader<EXTENT>::operator SQLiteIndexHeader()
    {
    SQLiteIndexHeader header;
    header.m_rootNodeBlockID = m_rootNodeBlockID.IsValid() ? m_rootNodeBlockID.m_integerID : -1;
    header.m_balanced = m_balanced;
    header.m_depth = m_depth;
    header.m_terrainDepth = m_terrainDepth;
    header.m_HasMaxExtent = m_HasMaxExtent;
    header.m_MaxExtent = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_MaxExtent), ExtentOp<EXTENT>::GetYMin(m_MaxExtent), ExtentOp<EXTENT>::GetZMin(m_MaxExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_MaxExtent), ExtentOp<EXTENT>::GetYMax(m_MaxExtent), ExtentOp<EXTENT>::GetZMax(m_MaxExtent));
    header.m_numberOfSubNodesOnSplit = m_numberOfSubNodesOnSplit;
    header.m_singleFile = m_singleFile;
    header.m_SplitTreshold = m_SplitTreshold;
    header.m_textured = m_textured;
    header.m_isTerrain = m_isTerrain;
    return header;
    }

template <class EXTENT> SMIndexNodeHeader<EXTENT>::~SMIndexNodeHeader()
    {
    if (nullptr != m_meshComponents) delete[] m_meshComponents;
    }

template <class EXTENT> SMIndexNodeHeader<EXTENT>& SMIndexNodeHeader<EXTENT>::operator=(const SQLiteNodeHeader& nodeHeader)
    {
    m_arePoints3d = nodeHeader.m_arePoints3d;
    m_isTextured = nodeHeader.m_isTextured;
    m_contentExtentDefined = nodeHeader.m_contentExtentDefined;
    m_contentExtent = ExtentOp<EXTENT>::Create(nodeHeader.m_contentExtent.low.x, nodeHeader.m_contentExtent.low.y, nodeHeader.m_contentExtent.low.z,
                                               nodeHeader.m_contentExtent.high.x, nodeHeader.m_contentExtent.high.y, nodeHeader.m_contentExtent.high.z);
    m_nodeExtent = ExtentOp<EXTENT>::Create(nodeHeader.m_nodeExtent.low.x, nodeHeader.m_nodeExtent.low.y, nodeHeader.m_nodeExtent.low.z,
                                            nodeHeader.m_nodeExtent.high.x, nodeHeader.m_nodeExtent.high.y, nodeHeader.m_nodeExtent.high.z);
    if (nodeHeader.m_graphID != SQLiteNodeHeader::NO_NODEID) m_graphID = HPMBlockID(nodeHeader.m_graphID);
    m_filtered = nodeHeader.m_filtered;
    m_level = nodeHeader.m_level;
    m_nbFaceIndexes = nodeHeader.m_nbFaceIndexes;
    m_nbTextures = nodeHeader.m_nbTextures;
    m_nbUvIndexes = nodeHeader.m_nbUvIndexes;
    m_numberOfMeshComponents = nodeHeader.m_numberOfMeshComponents;
    m_meshComponents = nodeHeader.m_meshComponents;
    m_numberOfSubNodesOnSplit = nodeHeader.m_numberOfSubNodesOnSplit;
    if (nodeHeader.m_parentNodeID != SQLiteNodeHeader::NO_NODEID) m_parentNodeID = HPMBlockID(nodeHeader.m_parentNodeID);
    else m_parentNodeID = ISMStore::GetNullNodeID();
    if (nodeHeader.m_SubNodeNoSplitID != SQLiteNodeHeader::NO_NODEID) m_SubNodeNoSplitID = HPMBlockID(nodeHeader.m_SubNodeNoSplitID);
    if (nodeHeader.m_uvID != SQLiteNodeHeader::NO_NODEID) m_uvID = HPMBlockID(nodeHeader.m_uvID);
    m_totalCountDefined = nodeHeader.m_totalCountDefined;
    m_totalCount = nodeHeader.m_totalCount;
    m_nodeCount = nodeHeader.m_nodeCount;
    m_SplitTreshold = nodeHeader.m_SplitTreshold;
    m_clipSetsID.resize(nodeHeader.m_clipSetsID.size());
    for (auto& id : m_clipSetsID) if (nodeHeader.m_clipSetsID[&id - &m_clipSetsID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_clipSetsID[&id - &m_clipSetsID.front()]);
    m_textureID.resize(nodeHeader.m_textureID.size());
    for (auto& id : m_textureID) if (nodeHeader.m_textureID[&id - &m_textureID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_textureID[&id - &m_textureID.front()]);
    m_ptsIndiceID.resize(nodeHeader.m_ptsIndiceID.size());
    for (auto& id : m_ptsIndiceID) if (nodeHeader.m_ptsIndiceID[&id - &m_ptsIndiceID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_ptsIndiceID[&id - &m_ptsIndiceID.front()]);
    m_uvsIndicesID.resize(nodeHeader.m_uvsIndicesID.size());
    for (auto& id : m_uvsIndicesID) if (nodeHeader.m_uvsIndicesID[&id - &m_uvsIndicesID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_uvsIndicesID[&id - &m_uvsIndicesID.front()]);
    m_apSubNodeID.resize(nodeHeader.m_apSubNodeID.size());
    for (auto& id : m_apSubNodeID) if (nodeHeader.m_apSubNodeID[&id - &m_apSubNodeID.front()] != SQLiteNodeHeader::NO_NODEID) id = HPMBlockID(nodeHeader.m_apSubNodeID[&id - &m_apSubNodeID.front()]);
    for (size_t i = 0; i < 26; ++i)
        {
        for (auto& id : nodeHeader.m_apNeighborNodeID[i])
            if (id != SQLiteNodeHeader::NO_NODEID)
                m_apNeighborNodeID[i].push_back(HPMBlockID(id));
        }

    return *this;
    }

template <class EXTENT> SMIndexNodeHeader<EXTENT>::operator SQLiteNodeHeader()
    {
    SQLiteNodeHeader header;
    header.m_arePoints3d = m_arePoints3d;
    header.m_isTextured = m_isTextured;
    header.m_contentExtentDefined = m_contentExtentDefined;
    header.m_contentExtent = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_contentExtent), ExtentOp<EXTENT>::GetYMin(m_contentExtent), ExtentOp<EXTENT>::GetZMin(m_contentExtent),
                                            ExtentOp<EXTENT>::GetXMax(m_contentExtent), ExtentOp<EXTENT>::GetYMax(m_contentExtent), ExtentOp<EXTENT>::GetZMax(m_contentExtent));
    header.m_nodeExtent = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_nodeExtent), ExtentOp<EXTENT>::GetYMin(m_nodeExtent), ExtentOp<EXTENT>::GetZMin(m_nodeExtent),
                                         ExtentOp<EXTENT>::GetXMax(m_nodeExtent), ExtentOp<EXTENT>::GetYMax(m_nodeExtent), ExtentOp<EXTENT>::GetZMax(m_nodeExtent));
    header.m_graphID = m_graphID.IsValid() ? m_graphID.m_integerID : -1;
    header.m_filtered = m_filtered;
    header.m_level = m_level;
    header.m_nbFaceIndexes = m_nbFaceIndexes;
    header.m_nbTextures = m_nbTextures;
    header.m_nbUvIndexes = m_nbUvIndexes;
    header.m_numberOfMeshComponents = m_numberOfMeshComponents;
    header.m_meshComponents = m_meshComponents;
    header.m_numberOfSubNodesOnSplit = m_numberOfSubNodesOnSplit;
    header.m_parentNodeID = m_parentNodeID.IsValid() && m_parentNodeID != ISMStore::GetNullNodeID() ? m_parentNodeID.m_integerID : -1;
    header.m_SubNodeNoSplitID = m_SubNodeNoSplitID.IsValid() && m_SubNodeNoSplitID != ISMStore::GetNullNodeID() ? m_SubNodeNoSplitID.m_integerID : -1;
    header.m_uvID = m_uvID.IsValid() ? m_uvID.m_integerID : -1;
    header.m_totalCountDefined = m_totalCountDefined;
    header.m_totalCount = m_totalCount;
    header.m_SplitTreshold = m_SplitTreshold;
    header.m_clipSetsID.resize(m_clipSetsID.size());
    header.m_nodeCount = m_nodeCount;
    for (auto& id : m_clipSetsID) header.m_clipSetsID[&id - &m_clipSetsID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
    header.m_textureID.resize(m_textureID.size());
    for (auto& id : m_textureID) header.m_textureID[&id - &m_textureID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
    header.m_ptsIndiceID.resize(m_ptsIndiceID.size());
    for (auto& id : m_ptsIndiceID) header.m_ptsIndiceID[&id - &m_ptsIndiceID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
    header.m_uvsIndicesID.resize(m_uvsIndicesID.size());
    for (auto& id : m_uvsIndicesID) header.m_uvsIndicesID[&id - &m_uvsIndicesID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
    header.m_apSubNodeID.resize(m_apSubNodeID.size());
    for (auto& id : m_apSubNodeID) header.m_apSubNodeID[&id - &m_apSubNodeID.front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
    if (header.m_SubNodeNoSplitID != -1) header.m_apSubNodeID[0] = header.m_SubNodeNoSplitID;
    for (size_t i = 0; i < 26; ++i)
        {
        header.m_apNeighborNodeID[i].resize(m_apNeighborNodeID[i].size());
        for (auto& id : m_apNeighborNodeID[i]) header.m_apNeighborNodeID[i][&id - &m_apNeighborNodeID[i].front()] = id.IsValid() && id != ISMStore::GetNullNodeID() ? id.m_integerID : -1;
        }
    return header;
    }


template <class EXTENT> SMSQLiteStore<EXTENT>::SMSQLiteStore(SMSQLiteFilePtr database)
    {
    m_smSQLiteFile = database;
    }

template <class EXTENT> SMSQLiteStore<EXTENT>::~SMSQLiteStore()
    {
    // We didn't want to close it. It's ScalableMesh which did this stuff now (and if SMSQLiteFilePtr is destroy, close is automatically called)
    //m_smSQLiteFile->Close();
    }

template <class EXTENT> uint64_t SMSQLiteStore<EXTENT>::GetNextID() const
    {
    //return m_smSQLiteFile->GetLastInsertRowId(); // This only works if last insert was performed on the same database connection
    return m_smSQLiteFile->GetLastNodeId();
    }

template <class EXTENT> void SMSQLiteStore<EXTENT>::Close()
    {
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize)
    {
    if (indexHeader == nullptr) return 0;
    SQLiteIndexHeader header = *indexHeader;
    m_smSQLiteFile->SetMasterHeader(header);
    return true;
    }

template <class EXTENT> size_t SMSQLiteStore<EXTENT>::LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize)
    {
    if (indexHeader == nullptr) return 0;
    SQLiteIndexHeader header;
    if (!m_smSQLiteFile->GetMasterHeader(header)) return 0;
    if (header.m_rootNodeBlockID == SQLiteNodeHeader::NO_NODEID) return 0;
    *indexHeader = header;
    return sizeof(*indexHeader);
    }

template <class EXTENT> size_t SMSQLiteStore<EXTENT>::StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID)
    {
    if (header == nullptr) return 0;
    if (header->m_ptsIndiceID.size() > 0)header->m_ptsIndiceID[0] = blockID;
    SQLiteNodeHeader nodeHeader = *header;
    if (header->m_SubNodeNoSplitID.IsValid() && !header->m_apSubNodeID[0].IsValid())
        nodeHeader.m_apSubNodeID[0] = nodeHeader.m_SubNodeNoSplitID;
    nodeHeader.m_nodeID = blockID.m_integerID;
    nodeHeader.m_graphID = nodeHeader.m_nodeID;
    m_smSQLiteFile->SetNodeHeader(nodeHeader);
    return sizeof(header);
    }

template <class EXTENT> size_t SMSQLiteStore<EXTENT>::LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID)
    {
    if (header == nullptr) return 0;
    SQLiteNodeHeader nodeHeader;
    if (header->m_meshComponents != nullptr) delete[] header->m_meshComponents;
    nodeHeader.m_nodeID = blockID.m_integerID;
    if (!m_smSQLiteFile->GetNodeHeader(nodeHeader)) return 1;
    //nodeHeader.m_nbPoints = GetBlockDataCount(blockID);
    *header = nodeHeader;
    header->m_IsLeaf = header->m_apSubNodeID.size() == 0 || (!header->m_apSubNodeID[0].IsValid());
    header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID.size() > 1 && header->m_apSubNodeID[1].IsValid());
    if (!header->m_IsLeaf && !header->m_IsBranched)
        {
        header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];
        header->m_apSubNodeID[0] = HPMBlockID();
        }
    //if (header->m_ptsIndiceID.size() > 0)header->m_ptsIndiceID[0] = blockID;
    if (header->m_isTextured)
        {
        header->m_uvID = blockID;
        header->m_ptsIndiceID[0] = HPMBlockID();
        }
    header->m_graphID = blockID;
    return sizeof(*header);
    }    

template <class EXTENT> RefCountedPtr<ISMNodeDataStore<DPoint3d, SMIndexNodeHeader<EXTENT>>> SMSQLiteStore<EXTENT>::GetNodeDataStore(SMIndexNodeHeader<EXTENT>* nodeHeader)
    {    
    RefCountedPtr<ISMNodeDataStore<DPoint3d, SMIndexNodeHeader<EXTENT>>> nodeDataStorePtr;
        
    nodeDataStorePtr = new SMSQLiteNodePointStore<DPoint3d, EXTENT>(nodeHeader, m_smSQLiteFile);

    return nodeDataStorePtr;    
    }

template <class POINT, class EXTENT> SMSQLiteNodePointStore<POINT, EXTENT>::SMSQLiteNodePointStore(SMIndexNodeHeader<EXTENT>* nodeHeader, /*ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>* dataStore,*/ SMSQLiteFilePtr& smSQLiteFile)
    : ISMNodeDataStore(nodeHeader/*, dataStore*/)
    {       
    m_smSQLiteFile = smSQLiteFile;            
    }

template <class POINT, class EXTENT> SMSQLiteNodePointStore<POINT, EXTENT>::~SMSQLiteNodePointStore()
    {            
    }

template <class POINT, class EXTENT> HPMBlockID SMSQLiteNodePointStore<POINT, EXTENT>::StoreNewBlock(POINT* DataTypeArray, size_t countData)
    {        
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(POINT));
    pi_uncompressedPacket.SetDataSize(countData*sizeof(POINT));
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
    bvector<uint8_t> ptData(pi_compressedPacket.GetDataSize());
    memcpy(&ptData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = SQLiteNodeHeader::NO_NODEID;
    m_smSQLiteFile->StorePoints(id, ptData, countData*sizeof(POINT));
    return HPMBlockID(id);
    }

template <class POINT, class EXTENT> HPMBlockID SMSQLiteNodePointStore<POINT, EXTENT>::StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(POINT));
    pi_uncompressedPacket.SetDataSize(countData*sizeof(POINT));
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
    bvector<uint8_t> ptData(pi_compressedPacket.GetDataSize());
    memcpy(&ptData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = blockID.m_integerID;
    m_smSQLiteFile->StorePoints(id, ptData, countData*sizeof(POINT));
    return HPMBlockID(id);
    }

template <class POINT, class EXTENT> size_t SMSQLiteNodePointStore<POINT, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
    {
    if(!blockID.IsValid()) return 0;
    return m_smSQLiteFile->GetNumberOfPoints(blockID.m_integerID) / sizeof(POINT);
    }

template <class POINT, class EXTENT> size_t SMSQLiteNodePointStore<POINT, EXTENT>::LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    if (!blockID.IsValid()) return 0;
    bvector<uint8_t> ptData;
    size_t uncompressedSize = 0;
    m_smSQLiteFile->GetPoints(blockID.m_integerID, ptData, uncompressedSize);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_compressedPacket.SetBuffer(&ptData[0], ptData.size());
    pi_compressedPacket.SetDataSize(ptData.size());
    pi_uncompressedPacket.SetBuffer(DataTypeArray, maxCountData*sizeof(POINT));
    pi_uncompressedPacket.SetBufferOwnership(false);
    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);

    return std::min(uncompressedSize, maxCountData*sizeof(POINT));
    }

template <class POINT, class EXTENT> bool SMSQLiteNodePointStore<POINT, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return false;
    }