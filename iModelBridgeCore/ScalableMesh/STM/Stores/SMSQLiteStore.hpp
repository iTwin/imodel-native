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
    dataStore = new SMSQLiteNodeDataStore<DPoint3d, EXTENT>(SMStoreDataType::Points, nodeHeader, m_smSQLiteFile);

    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                
    assert(dataType == SMStoreDataType::TriPtIndices || dataType == SMStoreDataType::TriUvIndices);
    
    dataStore = new SMSQLiteNodeDataStore<int32_t, EXTENT>(dataType, nodeHeader, m_smSQLiteFile);
                    
    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {                
    dataStore = new SMSQLiteNodeDataStore<DPoint2d, EXTENT>(SMStoreDataType::UvCoords, nodeHeader, m_smSQLiteFile);
    return true;    
    }


//Multi-items loading store
template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {                
    dataStore = new SMSQLiteNodeDataStore<PointAndTriPtIndicesBase, EXTENT>(SMStoreDataType::PointAndTriPtIndices, nodeHeader, m_smSQLiteFile);
    return true;    
    }


template <class DATATYPE, class EXTENT> SMSQLiteNodeDataStore<DATATYPE, EXTENT>::SMSQLiteNodeDataStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader, /*ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>* dataStore,*/ SMSQLiteFilePtr& smSQLiteFile)    
    {       
    m_smSQLiteFile = smSQLiteFile;
    m_nodeHeader = nodeHeader;
    m_dataType = dataType;
    }

template <class DATATYPE, class EXTENT> SMSQLiteNodeDataStore<DATATYPE, EXTENT>::~SMSQLiteNodeDataStore()
    {            
    }

template <class DATATYPE, class EXTENT> HPMBlockID SMSQLiteNodeDataStore<DATATYPE, EXTENT>::StoreNewBlock(DATATYPE* DataTypeArray, size_t countData)
    {     
    int64_t id = SQLiteNodeHeader::NO_NODEID;

    if (m_dataType == SMStoreDataType::PointAndTriPtIndices)
        {
        assert(!"Not done yet");
        }
    else
        {
        HCDPacket pi_uncompressedPacket, pi_compressedPacket;
        pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(DATATYPE));
        pi_uncompressedPacket.SetDataSize(countData*sizeof(DATATYPE));
        WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
        bvector<uint8_t> nodeData(pi_compressedPacket.GetDataSize());
        memcpy(&nodeData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
        
        switch (m_dataType)
            {
            case SMStoreDataType::Points : 
                m_smSQLiteFile->StorePoints(id, nodeData, countData*sizeof(DATATYPE));
                break;
            case SMStoreDataType::TriPtIndices : 
                m_smSQLiteFile->StoreIndices(id, nodeData, countData*sizeof(DATATYPE));            
                break;            
            case SMStoreDataType::UvCoords : 
                m_smSQLiteFile->StoreUVs(id, nodeData, countData*sizeof(DATATYPE));
                break;
            case SMStoreDataType::TriUvIndices : 
                m_smSQLiteFile->StoreUVIndices(id, nodeData, countData*sizeof(DATATYPE));                
                break;                                
            default : 
                assert(!"Unsupported type");
                break;
            }
        }
        
    return HPMBlockID(id);
    }

template <class DATATYPE, class EXTENT> HPMBlockID SMSQLiteNodeDataStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices);

    if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(DATATYPE));
    pi_uncompressedPacket.SetDataSize(countData*sizeof(DATATYPE));
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
    bvector<uint8_t> nodeData(pi_compressedPacket.GetDataSize());
    memcpy(&nodeData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = blockID.m_integerID;
    
    switch (m_dataType)
        {
        case SMStoreDataType::Points : 
            m_smSQLiteFile->StorePoints(id, nodeData, countData*sizeof(DATATYPE));            
            break;
        case SMStoreDataType::TriPtIndices : 
            m_smSQLiteFile->StoreIndices(id, nodeData, countData*sizeof(DATATYPE));                        
            break;
        case SMStoreDataType::UvCoords : 
            m_smSQLiteFile->StoreUVs(id, nodeData, countData*sizeof(DATATYPE));
            break;
        case SMStoreDataType::TriUvIndices : 
            m_smSQLiteFile->StoreUVIndices(id, nodeData, countData*sizeof(DATATYPE));                            
            break;            
        default : 
            assert(!"Unsupported type");
            break;
        }

    return HPMBlockID(id);
    }

template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
    {
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices);
    
    return GetBlockDataCount(blockID, m_dataType);    
    }


template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const
    {    
    size_t blockDataCount = 0;

    switch (dataType)
        {
        case SMStoreDataType::Points : 
            blockDataCount = m_nodeHeader->m_nodeCount;            
            break;
        case SMStoreDataType::TriPtIndices : 
            blockDataCount = m_nodeHeader->m_nbFaceIndexes;            
            break;
        case SMStoreDataType::UvCoords : 
            blockDataCount = m_smSQLiteFile->GetNumberOfUVs(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::TriUvIndices :             
            blockDataCount = m_smSQLiteFile->GetNumberOfUVIndices(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        default : 
            assert(!"Unsupported type");
            break;
        }

    return blockDataCount;    
    }

template <class DATATYPE, class EXTENT> void SMSQLiteNodeDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) 
    {
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices);

    ModifyBlockDataCount(blockID, countDelta, m_dataType);
    }

template <class DATATYPE, class EXTENT> void SMSQLiteNodeDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) 
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

        //MST_TS
        case SMStoreDataType::UvCoords :
        case SMStoreDataType::TriUvIndices :             
            break;
        default : 
            assert(!"Unsupported type");
            break;
        }    
    }

template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    if (!blockID.IsValid()) return 0;
    
    size_t nbBytes; 
    
    if (m_dataType == SMStoreDataType::PointAndTriPtIndices)
        {        
        PointAndTriPtIndicesBase* pData = (PointAndTriPtIndicesBase*)(DataTypeArray);
        assert(pData != 0 && pData->m_pointData != 0 && pData->m_indicesData != 0);

        bvector<uint8_t> ptsData;        
        //MST_TS - Count not used?
        size_t uncompressedSizePts = 0;
        bvector<uint8_t> indicesData;
        size_t uncompressedSizeIndices = 0;

        m_smSQLiteFile->GetPointsAndIndices(blockID.m_integerID, ptsData, uncompressedSizePts, indicesData, uncompressedSizeIndices);

        assert(ptsData.size() > 0 && indicesData.size() > 0);

        if (ptsData.size() > 0)
            {
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;        

            pi_compressedPacket.SetBuffer(&ptsData[0], ptsData.size());
            pi_compressedPacket.SetDataSize(ptsData.size());                        
            pi_uncompressedPacket.SetBuffer(pData->m_pointData, GetBlockDataCount(blockID, SMStoreDataType::Points) * sizeof(*pData->m_pointData));
            pi_uncompressedPacket.SetBufferOwnership(false);
            LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);   
            }        

        if (indicesData.size() > 0)
            {
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;        
            pi_compressedPacket.SetBuffer(&indicesData[0], indicesData.size());
            pi_compressedPacket.SetDataSize(indicesData.size());                        
            pi_uncompressedPacket.SetBuffer(pData->m_indicesData, GetBlockDataCount(blockID, SMStoreDataType::TriPtIndices) * sizeof(*pData->m_indicesData));
            pi_uncompressedPacket.SetBufferOwnership(false);
            LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);       
            }        

        nbBytes = 1*sizeof(DATATYPE);        
        }
    else
        {
        bvector<uint8_t> nodeData;
        size_t uncompressedSize = 0;

        switch (m_dataType)
            {
            case SMStoreDataType::Points : 
                m_smSQLiteFile->GetPoints(blockID.m_integerID, nodeData, uncompressedSize);            
                break;
            case SMStoreDataType::TriPtIndices : 
                m_smSQLiteFile->GetIndices(blockID.m_integerID, nodeData, uncompressedSize);
                break;
            case SMStoreDataType::UvCoords :
                m_smSQLiteFile->GetUVs(blockID.m_integerID, nodeData, uncompressedSize);
                break;
            case SMStoreDataType::TriUvIndices :             
                m_smSQLiteFile->GetUVIndices(blockID.m_integerID, nodeData, uncompressedSize);
                break;
            default : 
                assert(!"Unsupported type");
                break;
            }    
    
        HCDPacket pi_uncompressedPacket, pi_compressedPacket;
        pi_compressedPacket.SetBuffer(&nodeData[0], nodeData.size());
        pi_compressedPacket.SetDataSize(nodeData.size());
        pi_uncompressedPacket.SetBuffer(DataTypeArray, maxCountData*sizeof(DATATYPE));
        pi_uncompressedPacket.SetBufferOwnership(false);
        LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);

        nbBytes = std::min(uncompressedSize, maxCountData*sizeof(DATATYPE));        
        }

    return nbBytes;
    }

template <class DATATYPE, class EXTENT> bool SMSQLiteNodeDataStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return false;
    }