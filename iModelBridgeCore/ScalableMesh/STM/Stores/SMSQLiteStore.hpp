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

template <class EXTENT> bool SMSQLiteStore<EXTENT>::SetProjectFilesPath(BeFileName& projectFilesPath)
    {
    if (m_projectFilesPath.length() > 0)
        return false;
    
    m_projectFilesPath = projectFilesPath;

    //NEEDS_WORK_SM : Ugly, load/creation of the project files should be done explicitly
    //Force the opening/creation of project file in main thread to avoid global mutex.
    GetSisterSQLiteFile(SMStoreDataType::DiffSet);        
    GetSisterSQLiteFile(SMStoreDataType::Skirt);        

    return true; 
    }    

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                   
    SMSQLiteFilePtr sqlFilePtr;

    if (dataType == SMStoreDataType::Skirt || dataType == SMStoreDataType::ClipDefinition)
        {
        sqlFilePtr = GetSisterSQLiteFile(dataType);
        }
    else
        {
        sqlFilePtr = m_smSQLiteFile;
        }

    assert(sqlFilePtr.IsValid());

    dataStore = new SMSQLiteNodeDataStore<DPoint3d, EXTENT>(dataType, nodeHeader, sqlFilePtr);

    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {   
    if (m_projectFilesPath.empty())
        return false; 

    SMSQLiteFilePtr sqliteFilePtr = GetSisterSQLiteFile(SMStoreDataType::DiffSet);
    assert(sqliteFilePtr.IsValid() == true);    

    dataStore = new SMSQLiteNodeDataStore<DifferenceSet, EXTENT>(SMStoreDataType::DiffSet, nodeHeader, sqliteFilePtr);

    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetSisterSQLiteFileName(WString& sqlFileName, SMStoreDataType dataType)
    {    
    switch (dataType)
        {
        case SMStoreDataType::LinearFeature :
            {
            Utf8String dbFileName;         
            bool result = m_smSQLiteFile->GetFileName(dbFileName); 
            assert(result == true);

            WString name;
            name.AssignUtf8(dbFileName.c_str());

            sqlFileName = name.append(L"_feature"); //temporary file, deleted after generation
            }
            return true;
            break;

        case SMStoreDataType::DiffSet :
            sqlFileName = m_projectFilesPath;
            sqlFileName.append(L"_clips"); 
            return true;
            break;
        case SMStoreDataType::ClipDefinition :
        case SMStoreDataType::Skirt :
            sqlFileName = m_projectFilesPath;
            sqlFileName.append(L"_clipDefinitions"); 
            return true;
            break;

        default : 
            assert(!"Unknown data type");
            break;
        }    

    return false;
    }


template <class EXTENT> SMSQLiteFilePtr SMSQLiteStore<EXTENT>::GetSisterSQLiteFile(SMStoreDataType dataType)
    {    
    SMSQLiteFilePtr sqlFilePtr; 

    switch (dataType)
        {
        case SMStoreDataType::LinearFeature : 
            {
            if (!m_smFeatureSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);
                        
                _wremove(sqlFileName.c_str());

                m_smFeatureSQLiteFile = new SMSQLiteFile();
                m_smFeatureSQLiteFile->Create(sqlFileName); 
                }

            sqlFilePtr = m_smFeatureSQLiteFile;        
            }
            break;
        
        case SMStoreDataType::DiffSet :         
            {
            if (!m_smClipSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);  

                StatusInt status;
                m_smClipSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status);

                if (status == 0)
                    {                    
                    m_smClipSQLiteFile->Create(sqlFileName); 
                    }                                                                
                }

            sqlFilePtr = m_smClipSQLiteFile;                   
            }
            break;

        case SMStoreDataType::ClipDefinition :
        case SMStoreDataType::Skirt : 
            {
            if (!m_smClipDefinitionSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);  

                StatusInt status;
                m_smClipDefinitionSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status);

                if (status == 0)
                    {                    
                    m_smClipDefinitionSQLiteFile->Create(sqlFileName); 
                    }                                                                
                }  

            sqlFilePtr = m_smClipDefinitionSQLiteFile;                   
            }
            break;            

        default : 
            assert(!"Unknown datatype");
            break;
        }
        
    return sqlFilePtr;
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                
#ifdef WIP_MESH_IMPORT
    assert(dataType == SMStoreDataType::TriPtIndices || dataType == SMStoreDataType::TriUvIndices || dataType == SMStoreDataType::LinearFeature || dataType == SMStoreDataType::MeshParts);
#else
    assert(dataType == SMStoreDataType::TriPtIndices || dataType == SMStoreDataType::TriUvIndices || dataType == SMStoreDataType::LinearFeature);
#endif

    SMSQLiteFilePtr sqliteFilePtr;      

    if (dataType == SMStoreDataType::LinearFeature)
        {
        sqliteFilePtr = GetSisterSQLiteFile(SMStoreDataType::LinearFeature);            
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

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                        
    dataStore = new SMSQLiteNodeDataStore<Byte, EXTENT>(dataType, nodeHeader, m_smSQLiteFile);
                    
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

template <class DATATYPE, class EXTENT> HPMBlockID SMSQLiteNodeDataStore<DATATYPE, EXTENT>::StoreTexture(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    if (countData == 0)
        {
        int64_t id = blockID.IsValid() ? blockID.m_integerID : SQLiteNodeHeader::NO_NODEID;
        bvector<uint8_t> texData;
        m_smSQLiteFile->StoreTexture(id, texData, 0); // We store the number of bytes of the uncompressed image, ignoring the bytes used to store width, height, number of channels and format
        return HPMBlockID(id);
        }
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
    
int32_t* SerializeDiffSet(size_t& countAsPts, DifferenceSet* DataTypeArray, size_t countData)
    {
    void** serializedSet = new void*[countData];
    countAsPts = 0;
    size_t countAsBytes = 0;
    size_t* ct = new size_t[countData];

    for (size_t i = 0; i < countData; i++)
        {
        ct[i] = DataTypeArray[i].WriteToBinaryStream(serializedSet[i]);
        countAsBytes += ct[i];
        countAsPts += (size_t)(ceil((float)ct[i] / sizeof(int32_t)));
        }
    //countAsPts = (size_t)(ceil((float)countAsBytes / sizeof(int32_t)));
    size_t nOfInts = (size_t)(ceil(((float)sizeof(size_t) / sizeof(int32_t))));
    int32_t* ptArray = new int32_t[countAsPts + countData + nOfInts];
    memcpy(ptArray, &countData, sizeof(size_t));
    size_t offset = sizeof(size_t);
    for (size_t i = 0; i < countData; i++)
        {
        ptArray[(size_t)(ceil(((float)offset / sizeof(int32_t))))] = (int32_t)ct[i];
        offset = (size_t)(ceil(((float)offset / sizeof(int32_t))))*sizeof(int32_t);
        offset += sizeof(int32_t);
        memcpy((char*)ptArray + offset, serializedSet[i], ct[i]);
        offset += ct[i];
        free(serializedSet[i]);
        }
    delete[] serializedSet;
    delete[] ct;
    return ptArray;
    }

template <class DATATYPE, class EXTENT> HPMBlockID SMSQLiteNodeDataStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices);        

    //Special case
    if (m_dataType == SMStoreDataType::Texture)
        {
        return StoreTexture(DataTypeArray, countData, blockID);
        }

    bool needCompression = true;

    size_t dataSize;
    void* dataBuffer; 

    if (m_dataType == SMStoreDataType::Graph)
        {
        assert(typeid(DATATYPE) == typeid(MTGGraph));
        dataSize = ((MTGGraph*)DataTypeArray)->WriteToBinaryStream(dataBuffer);        
        }
    else
    if (m_dataType == SMStoreDataType::DiffSet)
        {
        size_t countAsPts;
        dataBuffer = SerializeDiffSet(countAsPts, (DifferenceSet*)DataTypeArray, countData);        
        dataSize = countAsPts*sizeof(int) + countData*sizeof(int) + sizeof(size_t);                
        needCompression = false;
        }
    else
        {
        dataSize = countData*sizeof(DATATYPE);
        dataBuffer = DataTypeArray;
        }

    bvector<uint8_t> nodeData;

    if (needCompression)
        {        
        HCDPacket pi_uncompressedPacket, pi_compressedPacket;
        pi_uncompressedPacket.SetBuffer(dataBuffer, dataSize);
        pi_uncompressedPacket.SetDataSize(dataSize);
        WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
        nodeData.resize(pi_compressedPacket.GetDataSize());        
        memcpy(&nodeData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
        }
    else
        {
        nodeData.resize(dataSize);
        memcpy(&nodeData[0], dataBuffer, dataSize);
        }

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
        case SMStoreDataType::DiffSet :             
            m_smSQLiteFile->StoreDiffSet(id, nodeData, dataSize);
            delete [] dataBuffer;
            break;             
        case SMStoreDataType::Skirt :             
            m_smSQLiteFile->StoreSkirtPolygon(id, nodeData, countData*sizeof(DATATYPE));            
            break;
        case SMStoreDataType::ClipDefinition :
            m_smSQLiteFile->StoreClipPolygon(id, nodeData, countData*sizeof(DATATYPE));
            break;                
#ifdef WIP_MESH_IMPORT
        case SMStoreDataType::MeshParts:
            m_smSQLiteFile->StoreMeshParts(id, nodeData, countData*sizeof(DATATYPE));
            break;
        case SMStoreDataType::Metadata:
            m_smSQLiteFile->StoreMetadata(id, nodeData, countData*sizeof(DATATYPE));
            break;
#endif
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
            blockDataCount = m_smSQLiteFile->GetTextureByteCount(blockID.m_integerID);
            if(blockDataCount > 0) blockDataCount += 3 * sizeof(int);
            break;
        case SMStoreDataType::UvCoords : 
            blockDataCount = m_smSQLiteFile->GetNumberOfUVs(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::DiffSet : 
            {   
            bvector<uint8_t> nodeData;
            size_t uncompressedSize = 0;  
            m_smSQLiteFile->GetDiffSet(blockID.m_integerID, nodeData, uncompressedSize);            
            
            if (uncompressedSize == 0) 
                blockDataCount = 0;
            else
                memcpy(&blockDataCount , &nodeData[0], sizeof(size_t)); //NEEDS_WORK_SM : never persist size_t, change that to uint64_t instead                
            }
            break;
        case SMStoreDataType::Skirt : 
            blockDataCount = m_smSQLiteFile->GetSkirtPolygonByteCount(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::ClipDefinition :
            blockDataCount = m_smSQLiteFile->GetClipPolygonByteCount(blockID.m_integerID) / sizeof(DATATYPE);
            break;
#ifdef WIP_MESH_IMPORT
        case SMStoreDataType::MeshParts:
            blockDataCount = m_smSQLiteFile->GetNumberOfMeshParts(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::Metadata:
            blockDataCount=  m_smSQLiteFile->GetNumberOfMetadataChars(blockID.m_integerID) / sizeof(DATATYPE);
            break;
#endif
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
case SMStoreDataType::DiffSet :
case SMStoreDataType::Skirt :  
        case SMStoreDataType::LinearFeature :
        case SMStoreDataType::UvCoords :
        case SMStoreDataType::TriUvIndices :
        case SMStoreDataType::Texture :
#ifdef WIP_MESH_IMPORT
        case SMStoreDataType::MeshParts:
        case SMStoreDataType::Metadata:
#endif
            break;
        default : 
            assert(!"Unsupported type");
            break;
        }    
    }


template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::DecompressTextureData(bvector<uint8_t>& texData, DATATYPE* DataTypeArray, size_t uncompressedSize)
    {
    assert(sizeof(DATATYPE) == 1);

    HCDPacket pi_uncompressedPacket((Byte*)DataTypeArray + sizeof(int) * 3, uncompressedSize, uncompressedSize);    
    HCDPacket pi_compressedPacket;
    pi_compressedPacket.SetBuffer(texData.data() + 4 * sizeof(int), texData.size() - 4 * sizeof(int));
    pi_compressedPacket.SetDataSize(texData.size() - 4 * sizeof(int));
    
    int *pHeader = (int*)(texData.data());
    int w = pHeader[0];
    int h = pHeader[1];
    int nOfChannels = pHeader[2];
    //int format = pHeader[3]; // The format is not used yet, but is may be useful in the future to support other compression than JPEG
    LoadJpegCompressedPacket(pi_compressedPacket, pi_uncompressedPacket, w, h, nOfChannels);    
    ((int*)DataTypeArray)[0] = w;
    ((int*)DataTypeArray)[1] = h;
    ((int*)DataTypeArray)[2] = nOfChannels;
    return uncompressedSize + sizeof(int) * 3;
    }


template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    if (!blockID.IsValid() || maxCountData == 0) return 0;
    
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

    bvector<uint8_t> nodeData;
    size_t uncompressedSize = 0;
    this->GetCompressedBlock(nodeData, uncompressedSize, blockID);

    //Special case
    if (m_dataType == SMStoreDataType::Texture)
        {        
        if (nodeData.size() == 0) return 0;
        assert(uncompressedSize + sizeof(int) * 3 == maxCountData);
        return DecompressTextureData(nodeData, DataTypeArray, uncompressedSize);
        }
    if (m_dataType == SMStoreDataType::DiffSet)
        {
        if (uncompressedSize == 0) return 1;
        size_t offset = (size_t)ceil(sizeof(size_t));        
        size_t ct = 0;

        size_t dataCount = 0;
        memcpy(&dataCount, &nodeData[0], sizeof(size_t));   
        assert(dataCount > 0);

        while (offset + 1 < nodeData.size() && *((int32_t*)&nodeData[offset]) > 0 && ct < dataCount)
            {
            //The pooled vectors don't initialize the memory they allocate. For complex datatypes with some logic in the constructor (like bvector),
            //this leads to undefined behavior when using the object. So we call the constructor on the allocated memory from the pool right here using placement new.
            DifferenceSet * diffSet = new(DataTypeArray + ct)DifferenceSet();
            size_t sizeOfCurrentSerializedSet = (size_t)*((int32_t*)&nodeData[offset]);
            diffSet->LoadFromBinaryStream(&nodeData[0] + offset + sizeof(int32_t), sizeOfCurrentSerializedSet);
            diffSet->upToDate = true;
            offset += sizeof(int32_t);
            offset += sizeOfCurrentSerializedSet;
            offset = ceil(((float)offset / sizeof(int32_t)))*sizeof(int32_t);
            ++ct;
            }        

        return nodeData.size();     
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

template <class DATATYPE, class EXTENT> void SMSQLiteNodeDataStore<DATATYPE, EXTENT>::GetCompressedBlock(bvector<uint8_t>& nodeData, size_t& uncompressedSize, HPMBlockID blockID)
    {
    switch (m_dataType)
        {        
        case SMStoreDataType::DiffSet : 
            m_smSQLiteFile->GetDiffSet(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::Graph : 
            m_smSQLiteFile->GetGraph(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::LinearFeature :
            m_smSQLiteFile->GetFeature(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::Points:
            m_smSQLiteFile->GetPoints(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::Skirt :
            m_smSQLiteFile->GetSkirtPolygon(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::TriPtIndices:
            m_smSQLiteFile->GetIndices(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::UvCoords:
            m_smSQLiteFile->GetUVs(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::TriUvIndices:
            m_smSQLiteFile->GetUVIndices(blockID.m_integerID, nodeData, uncompressedSize);
            break;            
#ifdef WIP_MESH_IMPORT
        case SMStoreDataType::MeshParts:
            m_smSQLiteFile->GetMeshParts(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        case SMStoreDataType::Metadata:
            m_smSQLiteFile->GetMetadata(blockID.m_integerID, nodeData, uncompressedSize);
            break;
#endif
        case SMStoreDataType::ClipDefinition :
            m_smSQLiteFile->GetClipPolygon(blockID.m_integerID, nodeData, uncompressedSize);           
            break;    
        case SMStoreDataType::Texture:
            m_smSQLiteFile->GetTexture(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        default:
            assert(!"Unsupported type");
            break;
        }
    }

template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::LoadCompressedBlock(bvector<DATATYPE>& DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    bvector<uint8_t> nodeData;
    size_t uncompressedSize = 0;
    this->GetCompressedBlock(nodeData, uncompressedSize, blockID);
    assert(uncompressedSize <= maxCountData*sizeof(DATATYPE));
    assert(nodeData.size() <= maxCountData*sizeof(DATATYPE));
    memcpy(DataTypeArray.data(), nodeData.data(), nodeData.size()*sizeof(DATATYPE));
    return nodeData.size();
    }

template <class DATATYPE, class EXTENT> bool SMSQLiteNodeDataStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return false;
    }


template <class DATATYPE, class EXTENT> bool SMSQLiteNodeDataStore<DATATYPE, EXTENT>::GetClipDefinitionExtOps(IClipDefinitionExtOpsPtr& clipDefinitionExOpsPtr)
    {
    if (m_dataType != SMStoreDataType::ClipDefinition)
        {
        assert(!"Unexpected call");
        return false;
        }

    clipDefinitionExOpsPtr = new SMSQLiteClipDefinitionExtOps(m_smSQLiteFile);

    return true;
    }
   