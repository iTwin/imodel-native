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

    if (m_smFeatureSQLiteFile.IsValid())
        {
        Utf8String dbFileName;         
        bool result = m_smFeatureSQLiteFile->GetFileName(dbFileName); 
        assert(result == true);
        assert(m_smFeatureSQLiteFile->GetRefCount() == 1);
        m_smFeatureSQLiteFile->Close();
        m_smFeatureSQLiteFile = 0;        
        WString dbFileNameW;
        dbFileNameW.AssignUtf8(dbFileName.c_str());
        _wremove(dbFileNameW.c_str());
        }
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
template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {                
    dataStore = new SMSQLiteNodeDataStore<DPoint3d, EXTENT>(SMStoreDataType::Points, nodeHeader, m_smSQLiteFile);

    return true;    
    }


template <class EXTENT> SMSQLiteFilePtr SMSQLiteStore<EXTENT>::GetFeatureSQLiteFile()
    {
    if (!m_smFeatureSQLiteFile.IsValid())
        {
        Utf8String dbFileName;         
        bool result = m_smSQLiteFile->GetFileName(dbFileName); 
        assert(result == true);

        WString name;
        name.AssignUtf8(dbFileName.c_str());

        WString featureFilePath = name.append(L"_feature"); //temporary file, deleted after generation
        _wremove(featureFilePath.c_str());

        m_smFeatureSQLiteFile = new SMSQLiteFile();
        m_smFeatureSQLiteFile->Create(featureFilePath); 
        }

    return m_smFeatureSQLiteFile;
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                
    assert(dataType == SMStoreDataType::TriPtIndices || dataType == SMStoreDataType::TriUvIndices || dataType == SMStoreDataType::LinearFeature);
    SMSQLiteFilePtr sqliteFilePtr;      

    if (dataType == SMStoreDataType::LinearFeature)
        {
        sqliteFilePtr = GetFeatureSQLiteFile();            
        assert(sqliteFilePtr.IsValid() == true);
        }
    else
        {
        sqliteFilePtr = m_smSQLiteFile;
        }
    
    dataStore = new SMSQLiteNodeDataStore<int32_t, EXTENT>(dataType, nodeHeader, sqliteFilePtr);
                    
    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {                        
    dataStore = new SMSQLiteNodeDataStore<MTGGraph, EXTENT>(SMStoreDataType::Graph, nodeHeader, m_smSQLiteFile);
                    
    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {                        
    dataStore = new SMSQLiteNodeDataStore<Byte, EXTENT>(SMStoreDataType::Texture, nodeHeader, m_smSQLiteFile);
                    
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
    assert(!"Should not be called");
   
    return HPMBlockID(-1);
    }


template <class DATATYPE, class EXTENT> HPMBlockID SMSQLiteNodeDataStore<DATATYPE, EXTENT>::StoreTexture(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(DataTypeArray + 3 * sizeof(int), countData - 3 * sizeof(int)); // The data block starts with 12 bytes of metadata, followed by pixel data
    pi_uncompressedPacket.SetDataSize(countData - 3 * sizeof(int));
    // Retrieve width, height and number of channels from the first 12 bytes of the data block
    int w = ((int*)DataTypeArray)[0];
    int h = ((int*)DataTypeArray)[1];
    int nOfChannels = ((int*)DataTypeArray)[2];
    int format = 0; // Keep an int to define the format and possible other options
    // Compress the image with JPEG
    WriteJpegCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, w, h, nOfChannels);
    // Create the compressed data block by storing width, height, number of channels and format before the compressed image block
    bvector<uint8_t> texData(4 * sizeof(int) + pi_compressedPacket.GetDataSize());
    int *pHeader = (int*)(texData.data());
    pHeader[0] = w;
    pHeader[1] = h;
    pHeader[2] = nOfChannels;
    pHeader[3] = format;
    memcpy(texData.data() + 4 * sizeof(int), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = blockID.IsValid() ? blockID.m_integerID : SQLiteNodeHeader::NO_NODEID;
    m_smSQLiteFile->StoreTexture(id, texData, pi_uncompressedPacket.GetDataSize()); // We store the number of bytes of the uncompressed image, ignoring the bytes used to store width, height, number of channels and format
    return HPMBlockID(id);
    }
    
template <class DATATYPE, class EXTENT> HPMBlockID SMSQLiteNodeDataStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices);    

    if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);

    //Special case
    if (m_dataType == SMStoreDataType::Texture)
        {
        return StoreTexture(DataTypeArray, countData, blockID);
        }

    size_t dataSize;
    void* dataBuffer; 

    if (m_dataType == SMStoreDataType::Graph)
        {
        assert(typeid(DATATYPE) == typeid(MTGGraph));
        dataSize = ((MTGGraph*)DataTypeArray)->WriteToBinaryStream(dataBuffer);        
        }
    else
        {
        dataSize = countData*sizeof(DATATYPE);
        dataBuffer = DataTypeArray;
        }

    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(dataBuffer, dataSize);
    pi_uncompressedPacket.SetDataSize(dataSize);
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
        case SMStoreDataType::TriUvIndices : 
            m_smSQLiteFile->StoreUVIndices(id, nodeData, countData*sizeof(DATATYPE));                            
            break;                    
        case SMStoreDataType::Graph : 
            m_smSQLiteFile->StoreGraph(id, nodeData, dataSize);
            free(dataBuffer);
            break;                                
        case SMStoreDataType::LinearFeature :
            m_smSQLiteFile->StoreFeature(id, nodeData, countData*sizeof(DATATYPE));
            break;
        case SMStoreDataType::UvCoords : 
            m_smSQLiteFile->StoreUVs(id, nodeData, countData*sizeof(DATATYPE));
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
        case SMStoreDataType::Graph : 
            return 1; 
            break;
        case SMStoreDataType::LinearFeature :
            m_smSQLiteFile->GetNumberOfFeaturePoints(blockID.m_integerID);
            break;
        case SMStoreDataType::Points : 
            blockDataCount = m_nodeHeader->m_nodeCount;            
            break;
        case SMStoreDataType::TriPtIndices : 
            blockDataCount = m_nodeHeader->m_nbFaceIndexes;            
            break;                  
        case SMStoreDataType::TriUvIndices :             
            blockDataCount = m_smSQLiteFile->GetNumberOfUVIndices(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::Texture :
            blockDataCount = m_smSQLiteFile->GetTextureByteCount(blockID.m_integerID) + 3 * sizeof(int);
            break;
        case SMStoreDataType::UvCoords : 
            blockDataCount = m_smSQLiteFile->GetNumberOfUVs(blockID.m_integerID) / sizeof(DATATYPE);
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
    assert(SMStoreDataType::Graph != m_dataType);

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
        case SMStoreDataType::LinearFeature :
        case SMStoreDataType::UvCoords :
        case SMStoreDataType::TriUvIndices :
        case SMStoreDataType::Texture :
            break;
        default : 
            assert(!"Unsupported type");
            break;
        }    
    }


template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::LoadTextureBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    assert(sizeof(DATATYPE) == 1);
    bvector<uint8_t> ptData;
    size_t uncompressedSize = 0;
    m_smSQLiteFile->GetTexture(blockID.m_integerID, ptData, uncompressedSize);

    assert(uncompressedSize + sizeof(int) * 3 == maxCountData);
    HCDPacket pi_uncompressedPacket((Byte*)DataTypeArray + sizeof(int) * 3, uncompressedSize, uncompressedSize);    
    HCDPacket pi_compressedPacket;
    pi_compressedPacket.SetBuffer(ptData.data() + 4 * sizeof(int), ptData.size() - 4 * sizeof(int));
    pi_compressedPacket.SetDataSize(ptData.size() - 4 * sizeof(int));
    
    int *pHeader = (int*)(ptData.data());
    int w = pHeader[0];
    int h = pHeader[1];
    int nOfChannels = pHeader[2];
    //int format = pHeader[3]; // The format is not used yet, but is may be useful in the future to support other compression than JPEG
    LoadJpegCompressedPacket(pi_compressedPacket, pi_uncompressedPacket, w, h, nOfChannels);    
    ((int*)DataTypeArray)[0] = w;
    ((int*)DataTypeArray)[1] = h;
    ((int*)DataTypeArray)[2] = nOfChannels;
    return maxCountData;    
    }


template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    if (!blockID.IsValid()) return 0;
    
#if 0 
    /*Multi item loading example
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

        return 1*sizeof(DATATYPE);        
        }
    */
#endif

    //Special case
    if (m_dataType == SMStoreDataType::Texture)
        {        
        return LoadTextureBlock(DataTypeArray, maxCountData, blockID);
        }

    bvector<uint8_t> nodeData;
    size_t uncompressedSize = 0;        

    switch (m_dataType)
        {        
        case SMStoreDataType::Graph : 
            m_smSQLiteFile->GetGraph(blockID.m_integerID, nodeData, uncompressedSize);

            if (uncompressedSize == 0)
                return 1; 
            break;
        case SMStoreDataType::LinearFeature :
            m_smSQLiteFile->GetFeature(blockID.m_integerID, nodeData, uncompressedSize);
            break;
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

    if (m_dataType == SMStoreDataType::Graph)
        {
        pi_uncompressedPacket.SetDataSize(uncompressedSize);        
        pi_uncompressedPacket.SetBuffer(new Byte[uncompressedSize], uncompressedSize);
        pi_uncompressedPacket.SetBufferOwnership(true);

        }
    else
        {
        assert(uncompressedSize == maxCountData*sizeof(DATATYPE));
        pi_uncompressedPacket.SetBuffer(DataTypeArray, maxCountData*sizeof(DATATYPE));
        pi_uncompressedPacket.SetBufferOwnership(false);
        }
    
    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);

    if (m_dataType == SMStoreDataType::Graph)
        {
        assert(typeid(DATATYPE) == typeid(MTGGraph));
        
        if(uncompressedSize > 0) 
            {
            MTGGraph * graph = new(DataTypeArray)MTGGraph(); //some of the memory managers call malloc but not the constructor            
            graph->LoadFromBinaryStream(pi_uncompressedPacket.GetBufferAddress(), uncompressedSize);
            }

        return 1;       
        }                

    return std::min(uncompressedSize, maxCountData*sizeof(DATATYPE));        
    }

template <class DATATYPE, class EXTENT> bool SMSQLiteNodeDataStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return false;
    }