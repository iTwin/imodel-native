#pragma once

#include "SMSQLiteStore.h"
#include "SMStreamedSourceStore.h"
#include "..\ScalableMeshSourcesPersistance.h"
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>
#include <ScalableMesh/IScalableMeshSourceVisitor.h>

#include <ScalableMesh/Import/SourceReference.h>

#include <ScalableMesh/Import/Source.h>
#include "..\RasterUtilities.h"

#include <ImagePP\all\h\HGF2DProjective.h>


template <class EXTENT> SMSQLiteStore<EXTENT>::SMSQLiteStore(SMSQLiteFilePtr database)
    : SMSQLiteSisterFile(database)
    {
    m_smSQLiteFile = database;   

    SourcesDataSQLite* sourcesData = new SourcesDataSQLite();
    m_smSQLiteFile->LoadSources(*sourcesData);

    WString wktStr;
    m_smSQLiteFile->GetWkt(wktStr);


    if (!wktStr.empty())
        {
        ISMStore::WktFlavor fileWktFlavor = GetWKTFlavor(&wktStr, wktStr);

        BaseGCS::WktFlavor wktFlavor;

        bool result = MapWktFlavorEnum(wktFlavor, fileWktFlavor);

        assert(result);

        SMStatus gcsFromWKTStatus = SMStatus::S_SUCCESS;
        GCS fileGCS(GetGCSFactory().Create(wktStr.c_str(), wktFlavor, gcsFromWKTStatus));
        if (!fileGCS.IsNull()) m_cs = fileGCS.GetGeoRef().GetBasePtr();
        }

    DocumentEnv sourceEnv(L"");
    bool success = BENTLEY_NAMESPACE_NAME::ScalableMesh::LoadSources(m_sources, *sourcesData, sourceEnv);
    assert(success == true);

    delete sourcesData;

    SMIndexMasterHeader<EXTENT> indexHeader;
    if (LoadMasterHeader(&indexHeader, sizeof(indexHeader)) > 0)
        {
        //we create the raster only once per dataset. Apparently there is some race condition if we do it in the render threads.
        if (indexHeader.m_textured == IndexTexture::Streaming) 
            {

            SQLiteNodeHeader nodeHeader;
            nodeHeader.m_nodeID = indexHeader.m_rootNodeBlockID.m_integerID;
            if (!m_smSQLiteFile->GetNodeHeader(nodeHeader))
                {
                assert(!"Dataset is empty");
                return;
                }
            SMIndexNodeHeader<EXTENT> header;
            header = nodeHeader;
            if (nodeHeader.m_parentNodeID == -1)  m_totalExtent = header.m_contentExtentDefined ? header.m_contentExtent : header.m_nodeExtent;

            const IDTMSource* rasterSource = nullptr;
            for (IDTMSourceCollection::const_iterator sourceIt = m_sources.Begin(), sourcesEnd = m_sources.End(); sourceIt != sourcesEnd;
                 ++sourceIt)
                {
                const IDTMSource& source = *sourceIt;
                if (source.GetSourceType() == DTM_SOURCE_DATA_IMAGE)
                    {
                    rasterSource = &source;
                    break;
                    }
                }

            if (rasterSource == nullptr)
                {
                assert(false && "Trying to use a streamed source but no raster source found!");
                return;
                }
            WString path;
            if (rasterSource->GetPath().StartsWith(L"http://"))
                {
                path = rasterSource->GetPath();
                }
            else
                {
                path = WString(L"file://") + rasterSource->GetPath();
                }

            //path = WString(L�http://www.bing.com/maps//�);
            path = WString(L"http://www.bing.com/maps/aerial/");

            DRange2d extent2d = DRange2d::From(m_totalExtent);
            m_raster = RasterUtilities::LoadRaster(m_streamingRasterFile, path, m_cs, extent2d);
            }
        }

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

    // keep a copy of the master header for future use...
    m_masterHeader = header;

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
    if (!m_smSQLiteFile->GetNodeHeader(nodeHeader))
        {
        // return a valid (empty) node header
        header->m_apSubNodeID.clear();
        header->m_nodeExtent = ExtentOp<EXTENT>::Create(0, 0, 0, 0, 0, 0);
        header->m_contentExtent = header->m_nodeExtent;
        header->m_totalCount = 0;
        return 1;
        }
    //nodeHeader.m_nbPoints = GetBlockDataCount(blockID);
    *header = nodeHeader;
    if (nodeHeader.m_parentNodeID == -1)  m_totalExtent = header->m_contentExtentDefined ? header->m_contentExtent : header->m_nodeExtent;
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
    return SMSQLiteSisterFile::SetProjectFilesPath(projectFilesPath);
    }    

template <class EXTENT> void SMSQLiteStore<EXTENT>::SaveProjectFiles()
    {
    __super::SaveSisterFiles();
    }

template <class EXTENT> void SMSQLiteStore<EXTENT>::CompactProjectFiles()
{
	__super::Compact();
}

template <class EXTENT> void SMSQLiteStore<EXTENT>::PreloadData(const bvector<DRange3d>& tileRanges)
    {        
    if (m_raster == nullptr)
        return;     

    for (auto& tileRange : tileRanges)
        {         
        HFCMatrix<3, 3> transfoMatrix;
        transfoMatrix[0][0] = (tileRange.high.x - tileRange.low.x) / 256;
        transfoMatrix[0][1] = 0;
        transfoMatrix[0][2] = tileRange.low.x;
        transfoMatrix[1][0] = 0;
        transfoMatrix[1][1] = -(tileRange.high.y - tileRange.low.y) / 256;
        transfoMatrix[1][2] = tileRange.high.y;
        transfoMatrix[2][0] = 0;
        transfoMatrix[2][1] = 0;
        transfoMatrix[2][2] = 1;

        HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

        HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

        if (pSimplifiedModel != 0)
        {
            pTransfoModel = pSimplifiedModel;
        }

        DPoint2d lowInPixels;
        DPoint2d highInPixels;

        pTransfoModel->ConvertInverse(tileRange.low.x, tileRange.low.y, &lowInPixels.x, &lowInPixels.y);
        pTransfoModel->ConvertInverse(tileRange.high.x, tileRange.high.y, &highInPixels.x, &highInPixels.y);

        HFCPtr<HGF2DCoordSys> coordSys(new HGF2DCoordSys(*pTransfoModel, m_raster->GetCoordSys()));

        HVEShape shape(lowInPixels.x, highInPixels.y, highInPixels.x, lowInPixels.y, coordSys);

        //HVEShape shape(total3dRange.low.x, total3dRange.low.y, total3dRange.high.x, total3dRange.high.y, coordSys);

        //HVEShape shape(total3dRange.low.x, total3dRange.low.y, total3dRange.high.x, total3dRange.high.y, m_raster->GetShape().GetCoordSys());

        uint32_t consumerID = 1;
        m_raster->SetLookAhead(shape, consumerID);
        }

#if 0 
    DRange3d total3dRange(DRange3d::NullRange());
    
    for (auto& range : tileRanges)
        { 
        total3dRange.Extend(range);
        }        

    HFCMatrix<3, 3> transfoMatrix;
    transfoMatrix[0][0] = (tileRanges[0].high.x - tileRanges[0].low.x) / 256;
    transfoMatrix[0][1] = 0;
    transfoMatrix[0][2] = total3dRange.low.x;
    transfoMatrix[1][0] = 0;
    transfoMatrix[1][1] = -(tileRanges[0].high.y - tileRanges[0].low.y) / 256;
    transfoMatrix[1][2] = total3dRange.high.y;
    transfoMatrix[2][0] = 0;
    transfoMatrix[2][1] = 0;
    transfoMatrix[2][2] = 1;    

    HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

    if (pSimplifiedModel != 0)
    {
        pTransfoModel = pSimplifiedModel;
    }

    DPoint2d lowInPixels; 
    DPoint2d highInPixels;

    pTransfoModel->ConvertInverse(total3dRange.low.x, total3dRange.low.y, &lowInPixels.x, &lowInPixels.y);
    pTransfoModel->ConvertInverse(total3dRange.high.x, total3dRange.high.y, &highInPixels.x, &highInPixels.y);

    HFCPtr<HGF2DCoordSys> coordSys(new HGF2DCoordSys(*pTransfoModel, m_raster->GetCoordSys()));

    HVEShape shape(lowInPixels.x, highInPixels.y, highInPixels.x, lowInPixels.y, coordSys);

    //HVEShape shape(total3dRange.low.x, total3dRange.low.y, total3dRange.high.x, total3dRange.high.y, coordSys);
            
    //HVEShape shape(total3dRange.low.x, total3dRange.low.y, total3dRange.high.x, total3dRange.high.y, m_raster->GetShape().GetCoordSys());


    

    uint32_t consumerID = 1;
    m_raster->SetLookAhead(shape, consumerID);
#endif
    }

template <class EXTENT> void SMSQLiteStore<EXTENT>::CancelPreloadData()
    {    
    if (m_streamingRasterFile != nullptr)
        { 
        HGFTileIDList blocks;

        m_streamingRasterFile->SetLookAhead(0, blocks, 0, false);

 //       m_streamingRasterFile->RequestLookAhead(99, blocks, false);
        }
/*
    uint32_t consumerID = 1;
    m_raster->SetLookAhead(shape, consumerID);
*/
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                   
    SMSQLiteFilePtr sqlFilePtr;
    
    sqlFilePtr = m_smSQLiteFile;    

    assert(sqlFilePtr.IsValid());

    dataStore = new SMSQLiteNodeDataStore<DPoint3d, EXTENT>(dataType, nodeHeader, sqlFilePtr);

    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetSisterNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, bool createSisterFile)
    {   
    if (!IsProjectFilesPathSet())
        return false; 

    SMSQLiteFilePtr sqliteFilePtr = GetSisterSQLiteFile(SMStoreDataType::DiffSet, createSisterFile);

    if (!sqliteFilePtr.IsValid())
        return false;
    
    dataStore = new SMSQLiteNodeDataStore<DifferenceSet, EXTENT>(SMStoreDataType::DiffSet, nodeHeader, sqliteFilePtr);

    return true;    
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
        sqliteFilePtr = GetSisterSQLiteFile(SMStoreDataType::LinearFeature, true);            
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
    SMSQLiteFilePtr sqliteFilePtr;


    sqliteFilePtr = GetSisterSQLiteFile(SMStoreDataType::Graph, true);
    assert(sqliteFilePtr.IsValid() == true);


    dataStore = new SMSQLiteNodeDataStore<MTGGraph, EXTENT>(SMStoreDataType::Graph, nodeHeader, sqliteFilePtr);
                    
    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                        
    if (m_masterHeader.m_textured == IndexTexture::Streaming)
        {
        dataStore = new SMStreamedSourceStore<Byte, EXTENT>(SMStoreDataType::Texture, nodeHeader, m_smSQLiteFile, m_totalExtent, m_raster);
        return true;
        }

    dataStore = new SMSQLiteNodeDataStore<Byte, EXTENT>(dataType, nodeHeader, m_smSQLiteFile);
                    
    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                
    assert(dataType == SMStoreDataType::UvCoords);
    dataStore = new SMSQLiteNodeDataStore<DPoint2d, EXTENT>(dataType, nodeHeader, m_smSQLiteFile);
    return true;    
    }


//Multi-items loading store
template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {                
    dataStore = new SMSQLiteNodeDataStore<PointAndTriPtIndicesBase, EXTENT>(SMStoreDataType::PointAndTriPtIndices, nodeHeader, m_smSQLiteFile);
    return true;    
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMTileMeshDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {
    dataStore = new SMSQLiteNodeDataStore<bvector<Byte>, EXTENT>(SMStoreDataType::Cesium3DTiles, nodeHeader, m_smSQLiteFile);
    return true;
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetNodeDataStore(ISMCesium3DTilesDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {
    dataStore = new SMSQLiteNodeDataStore<Cesium3DTilesBase, EXTENT>(SMStoreDataType::Cesium3DTiles, nodeHeader, m_smSQLiteFile);
    return true;
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetSisterNodeDataStore(ISMCoverageNameDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, bool createSisterFile)
    { 
    SMSQLiteFilePtr sqlFilePtr;
    
    sqlFilePtr = GetSisterSQLiteFile(SMStoreDataType::CoverageName, createSisterFile);

    if (!sqlFilePtr.IsValid())
        return false;
    
    dataStore = new SMSQLiteNodeDataStore<Utf8String, EXTENT>(SMStoreDataType::CoverageName, nodeHeader, sqlFilePtr);

    return true;
    }

template <class EXTENT> bool SMSQLiteStore<EXTENT>::GetSisterNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType, bool createSisterFile)
    {
    assert(dataType == SMStoreDataType::Skirt || dataType == SMStoreDataType::ClipDefinition || dataType == SMStoreDataType::CoveragePolygon);

    SMSQLiteFilePtr sqlFilePtr = GetSisterSQLiteFile(dataType, createSisterFile);

    if (!sqlFilePtr.IsValid())
        return false;

    dataStore = new SMSQLiteNodeDataStore<DPoint3d, EXTENT>(dataType, nodeHeader, sqlFilePtr);

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
    countData -= 3 * sizeof(int);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;

    // The static analyzer found a potential mismatch between sizeof and countof because of the statement 
    // DataTypeArray + 3 * sizeof(int) but there are other places where the same code is used and it has not complained about it...
    // However, I think the code is correct so I disable the warning here.
    #pragma warning( push )
    #pragma warning ( disable: 6305 )
    pi_uncompressedPacket.SetBuffer(DataTypeArray + 3 * sizeof(int), countData); // The data block starts with 12 bytes of metadata, followed by pixel data
    #pragma warning ( pop )
    pi_uncompressedPacket.SetDataSize(countData);
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

template<class DATATYPE, class EXTENT>
inline bool SMSQLiteNodeDataStore<DATATYPE, EXTENT>::IsCompressedType()
    {
    return m_dataType == SMStoreDataType::TextureCompressed;
    }
    
int32_t* SerializeDiffSet(size_t& countAsPts, DifferenceSet* DataTypeArray, size_t countData)
    {
    void** serializedSet = new void*[countData];
    countAsPts = 0;
    size_t countAsBytes = 0;
    uint64_t* ct = new uint64_t[countData];

    for (size_t i = 0; i < countData; i++)
        {
        ct[i] = DataTypeArray[i].WriteToBinaryStream(serializedSet[i]);
        countAsBytes += ct[i];
        countAsPts += (size_t)(ceil((float)ct[i] / sizeof(int32_t)));
        }
    //countAsPts = (size_t)(ceil((float)countAsBytes / sizeof(int32_t)));
    size_t nOfInts = (size_t)(ceil(((float)sizeof(size_t) / sizeof(int32_t))));
    int32_t* ptArray = new int32_t[countAsPts + countData + nOfInts];

	uint64_t newCountData = countData;
    memcpy(ptArray, &newCountData, sizeof(uint64_t));
    size_t offset = sizeof(uint64_t);
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
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices && m_dataType != SMStoreDataType::Cesium3DTiles);

    //Special case
    if (m_dataType == SMStoreDataType::Texture)
        {
        return StoreTexture(DataTypeArray, countData, blockID);
        }

    bool needCompression = true;

    size_t dataSize = 0;
    void* dataBuffer = nullptr; 

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
        //needCompression = false;
        }
    else
    if (m_dataType == SMStoreDataType::CoverageName)
        {
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
        case SMStoreDataType::CoveragePolygon:
            m_smSQLiteFile->StoreCoveragePolygon(id, nodeData, countData*sizeof(DATATYPE));
            break;
        case SMStoreDataType::CoverageName:
            {
            Utf8String name(*((Utf8String*)DataTypeArray));
            m_smSQLiteFile->StoreCoverageName(id, name, 1);
            }
            break;            
        default : 
            assert(!"Unsupported type");
            break;
        }

    return HPMBlockID(id);
    }

template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
    {
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices && m_dataType != SMStoreDataType::Cesium3DTiles);
    
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
        case SMStoreDataType::TextureCompressed:
            blockDataCount = m_smSQLiteFile->GetTextureCompressedByteCount(blockID.m_integerID);
            break;
        case SMStoreDataType::UvCoords :
            blockDataCount = m_smSQLiteFile->GetNumberOfUVs(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::DiffSet : 
            {   
            bvector<uint8_t> nodeData;
            size_t uncompressedSize = 0;  
            m_smSQLiteFile->GetDiffSet(blockID.m_integerID, nodeData, uncompressedSize);            
            
			HCDPacket pi_uncompressedPacket, pi_compressedPacket;
			pi_compressedPacket.SetBuffer(&nodeData[0], nodeData.size());
			pi_compressedPacket.SetDataSize(nodeData.size());
			pi_uncompressedPacket.SetDataSize(uncompressedSize);
			pi_uncompressedPacket.SetBuffer(new Byte[uncompressedSize], uncompressedSize);
			pi_uncompressedPacket.SetBufferOwnership(true);

			LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
            if (uncompressedSize == 0) 
                blockDataCount = 0;
			else
			    {
				uint64_t count;
				memcpy(&count, pi_uncompressedPacket.GetBufferAddress(), sizeof(uint64_t));
				blockDataCount = (size_t)count;
			    }
            }
            break;
        case SMStoreDataType::Skirt : 
            blockDataCount = m_smSQLiteFile->GetSkirtPolygonByteCount(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::ClipDefinition:
            blockDataCount = m_smSQLiteFile->GetClipPolygonByteCount(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::CoveragePolygon:
            blockDataCount = m_smSQLiteFile->GetCoveragePolygonByteCount(blockID.m_integerID) / sizeof(DATATYPE);
            break;
        case SMStoreDataType::CoverageName:
            blockDataCount = m_smSQLiteFile->GetCoverageNameByteCount(blockID.m_integerID) > 0 ? 1 : 0;
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
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices && m_dataType != SMStoreDataType::Cesium3DTiles);

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

    if (m_dataType == SMStoreDataType::CoverageName)
        {
        m_smSQLiteFile->GetCoverageName(blockID.m_integerID, (Utf8String*)DataTypeArray, uncompressedSize);
        return uncompressedSize;
        }

    this->GetCompressedBlock(nodeData, uncompressedSize, blockID);

    if (this->IsCompressedType())
        {
        memcpy(DataTypeArray, nodeData.data(), nodeData.size());
        return nodeData.size();
        }

    //Special case
    if (m_dataType == SMStoreDataType::Texture)
        {        
        if (nodeData.size() == 0) return 0;
        assert(uncompressedSize + sizeof(int) * 3 == maxCountData);
        return DecompressTextureData(nodeData, DataTypeArray, uncompressedSize);
        }
   /* else
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
            diffSet->upToDate = true;
            diffSet->LoadFromBinaryStream(&nodeData[0] + offset + sizeof(int32_t), sizeOfCurrentSerializedSet);
            offset += sizeof(int32_t);
            offset += sizeOfCurrentSerializedSet;
            offset = ceil(((float)offset / sizeof(int32_t)))*sizeof(int32_t);
            ++ct;
            }        

        return nodeData.size();     
        }    */    
        
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_compressedPacket.SetBuffer(&nodeData[0], nodeData.size());
    pi_compressedPacket.SetDataSize(nodeData.size());

    if (m_dataType == SMStoreDataType::Graph || m_dataType == SMStoreDataType::DiffSet)
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
	else
	{
		if (m_dataType == SMStoreDataType::DiffSet)
		{
			if (uncompressedSize == 0) return 1;
			uint64_t offset = (size_t)ceil(sizeof(uint64_t));
			size_t ct = 0;

			size_t dataCount = 0;
			memcpy(&dataCount, pi_uncompressedPacket.GetBufferAddress(), sizeof(uint64_t));
			assert(dataCount > 0);

			while (offset + 1 < (uint64_t)uncompressedSize && *((int32_t*)&pi_uncompressedPacket.GetBufferAddress()[offset]) > 0 && ct < dataCount)
			{
				//The pooled vectors don't initialize the memory they allocate. For complex datatypes with some logic in the constructor (like bvector),
				//this leads to undefined behavior when using the object. So we call the constructor on the allocated memory from the pool right here using placement new.
				DifferenceSet * diffSet = new(DataTypeArray + ct)DifferenceSet();
				uint64_t sizeOfCurrentSerializedSet = (uint64_t)*((int32_t*)&pi_uncompressedPacket.GetBufferAddress()[offset]);
				diffSet->upToDate = true;
				diffSet->LoadFromBinaryStream(&pi_uncompressedPacket.GetBufferAddress()[0] + offset + sizeof(int32_t), sizeOfCurrentSerializedSet);
				offset += sizeof(int32_t);
				offset += sizeOfCurrentSerializedSet;
				offset = ceil(((float)offset / sizeof(int32_t))) * sizeof(int32_t);
				++ct;
			}

			return uncompressedSize;
		}
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
            SMClipGeometryType geom;
            SMNonDestructiveClipType type;
            bool isActive;
            m_smSQLiteFile->GetClipPolygon(blockID.m_integerID, nodeData, uncompressedSize, geom, type, isActive);           
            break;  
        case SMStoreDataType::CoveragePolygon:
            m_smSQLiteFile->GetCoveragePolygon(blockID.m_integerID, nodeData, uncompressedSize);
            break;        
        case SMStoreDataType::Texture:
        case SMStoreDataType::TextureCompressed:
            m_smSQLiteFile->GetTexture(blockID.m_integerID, nodeData, uncompressedSize);
            break;
        default:
            assert(!"Unsupported type");
            break;
        }
    }

template <class DATATYPE, class EXTENT> size_t SMSQLiteNodeDataStore<DATATYPE, EXTENT>::LoadCompressedBlock(bvector<uint8_t>& DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    size_t uncompressedSize = 0;
    this->GetCompressedBlock(DataTypeArray, uncompressedSize, blockID);
    assert(uncompressedSize <= maxCountData*sizeof(DATATYPE));
    return DataTypeArray.size();
    }

template <class DATATYPE, class EXTENT> bool SMSQLiteNodeDataStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    switch (m_dataType)
        {
        case SMStoreDataType::CoveragePolygon:
            m_smSQLiteFile->DeleteCoveragePolygon(blockID.m_integerID);
            return true;
        case SMStoreDataType::ClipDefinition:
            m_smSQLiteFile->DeleteClipPolygon(blockID.m_integerID);
            return true;
        }
    return false;
    }


template <class DATATYPE, class EXTENT> bool SMSQLiteNodeDataStore<DATATYPE, EXTENT>::GetClipDefinitionExtOps(IClipDefinitionExtOpsPtr& clipDefinitionExOpsPtr)
    {
    if (m_dataType != SMStoreDataType::ClipDefinition && m_dataType != SMStoreDataType::CoveragePolygon)
        {
        assert(!"Unexpected call");
        return false;
        }

    clipDefinitionExOpsPtr = new SMSQLiteClipDefinitionExtOps(m_smSQLiteFile);

    return true;
    }
   