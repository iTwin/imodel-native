#pragma once

#include "SMSQLiteStore.h"



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

//template <class EXTENT> RefCountedPtr<ISMNodeDataStore<DPoint3d, SMIndexNodeHeader<EXTENT>>> SMSQLiteStore<EXTENT>::GetNodeDataStore(SMIndexNodeHeader<EXTENT>* nodeHeader)
template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMPointDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {                
    dataStore = new SMSQLiteNodePointStore<DPoint3d, EXTENT>(nodeHeader, m_smSQLiteFile);

    return true;    
    }

template <class POINT, class EXTENT> SMSQLiteNodePointStore<POINT, EXTENT>::SMSQLiteNodePointStore(SMIndexNodeHeader<EXTENT>* nodeHeader, /*ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>* dataStore,*/ SMSQLiteFilePtr& smSQLiteFile)    
    {       
    m_smSQLiteFile = smSQLiteFile;      
    m_nodeHeader = nodeHeader;
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
    /*
    if(!blockID.IsValid()) return 0;
    return m_smSQLiteFile->GetNumberOfPoints(blockID.m_integerID) / sizeof(POINT);
    */

    return m_nodeHeader->m_nodeCount;
    }

template <class POINT, class EXTENT> void SMSQLiteNodePointStore<POINT, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) 
    {
    assert((((int64_t)m_nodeHeader->m_nodeCount) + countDelta) > 0);
    m_nodeHeader->m_nodeCount += countDelta;    
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