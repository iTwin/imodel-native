/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ISMDataStore.h"
#include "SMSQLiteSisterFile.h"
#include "SMStoreUtils.h"
#include "../ScalableMeshSources.h"


struct SharedTransaction
{
private:
    bool m_transactionNeedsClosing;
    bool m_dbNeedsClosing;
    SMSQLiteFilePtr m_smSQLiteFile;
public:
    SharedTransaction(SMSQLiteFilePtr fileP, bool readonly, bool startTransaction = false);
    ~SharedTransaction();
};

template <class EXTENT> class SMSQLiteStore : public ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>, public SMSQLiteSisterFile
    {        
    private:

        SMSQLiteFilePtr m_smSQLiteFile;
        DRange3d m_totalExtent;
        GeoCoordinates::BaseGCSCPtr m_cs;
        IDTMSourceCollection m_sources;
        HFCPtr<HRFRASTERFILE> m_streamingRasterFile;
        HFCPtr<HRARASTER> m_raster;
        SMIndexMasterHeader<EXTENT> m_masterHeader;
        std::mutex                  m_preloadMutex;        
        IClipDefinitionDataProviderPtr m_clipProvider;


    public : 
    
        SMSQLiteStore(SMSQLiteFilePtr database);
            
        virtual ~SMSQLiteStore();       
                    
        virtual uint64_t GetNextID() const override;
            
        virtual void Close() override;
            
        virtual bool StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;
            
        virtual size_t LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;

        virtual SMNodeHeaderLocation GetNodeHeaderLocation(HPMBlockID blockID) override;

        virtual bool SetProjectFilesPath(BeFileName& projectFilesPath) override;

        virtual bool SetUseTempPath(bool useTempPath) override;

        virtual void SaveProjectFiles() override;

		virtual void CompactProjectFiles() override;

        virtual void PreloadData(const bvector<DRange3d>& tileRanges) override;
        
        virtual void CancelPreloadData() override;

        virtual void ComputeRasterTiles(bvector<SMRasterTile>& rasterTiles, const bvector<DRange3d>& tileRanges) override;

        virtual bool IsTextureAvailable() override;

		virtual bool DoesClipFileExist() const override;

        virtual void EraseClipFile() const override;

		virtual void SetClipDefinitionsProvider(const IClipDefinitionDataProviderPtr& provider) override;

		virtual void WriteClipDataToProjectFilePath() override;
                               
        virtual bool GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;        

        virtual bool GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;        

        virtual bool GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType = SMStoreDataType::Texture) override;

        virtual bool GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType = SMStoreDataType::UvCoords) override;        

        virtual bool GetSisterNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, bool createSisterFile) override;

        virtual bool GetSisterNodeDataStore(ISMCoverageNameDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, bool createSisterFile) override;

        virtual bool GetSisterNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType, bool createSisterFile) override;

        
        //Multi-items loading store
        virtual bool GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMTileMeshDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMCesium3DTilesDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual SMSQLiteFilePtr GetSQLiteFilePtr(SMStoreDataType dataType) override;

        static RefCountedPtr<ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>> Create (SMSQLiteFilePtr database)
        {
        return new SMSQLiteStore(database);
        }
        
    };

template <class DATATYPE, class EXTENT> class SMSQLiteNodeDataStore : public ISMNodeDataStore<DATATYPE> 
    {    
    private:

        ISMDataStoreTypePtr<EXTENT> m_dataStorePtr;
        SMSQLiteFilePtr             m_smSQLiteFile; //Could be the main file persisting the 3sm or a sister file.
        SMIndexNodeHeader<EXTENT>*  m_nodeHeader;
        SMStoreDataType             m_dataType;

        void   GetCompressedBlock(bvector<uint8_t>& nodeData, size_t& uncompressedSize, HPMBlockID blockID);

        size_t DecompressTextureData(bvector<uint8_t>& ptData, DATATYPE* DataTypeArray, size_t uncompressedSize);

        HPMBlockID StoreTexture(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID);

        HPMBlockID StoreTextureCompressed(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID);

        bool IsCompressedType();

    public:      
              
        SMSQLiteNodeDataStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader, SMSQLiteFilePtr& smSQLiteFile, ISMDataStoreTypePtr<EXTENT> dataStorePtr);
            
        virtual ~SMSQLiteNodeDataStore();                      
            
        virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;

        virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const override;
            
        virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;

        virtual size_t LoadCompressedBlock(bvector<uint8_t>& DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;

        virtual HPMBlockID StoreCompressedBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        virtual bool DestroyBlock(HPMBlockID blockID) override;         

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;        

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) override;

        virtual bool GetClipDefinitionExtOps(IClipDefinitionExtOpsPtr& clipDefinitionExOpsPtr) override;            

        virtual bool GetLinearFeaturesExtOps(ILinearFeaturesExtOpsPtr& linearFeaturesExtOpsPtr) override;
    };


class SMSQLiteClipDefinitionExtOps : public IClipDefinitionExtOps
    {
    private: 

        SMSQLiteFilePtr            m_smSQLiteFile;
       

    public :

        SMSQLiteClipDefinitionExtOps(SMSQLiteFilePtr& smSQLiteFile);

        virtual ~SMSQLiteClipDefinitionExtOps();

        virtual void GetMetadata(uint64_t id, double& importance, int& nDimensions) override;

        virtual void SetMetadata(uint64_t id, double importance, int nDimensions) override;

        virtual void GetAllIDs(bvector<uint64_t>& allIds) override;

        virtual void GetIsClipActive(uint64_t id, bool& isActive) override;

        virtual void GetClipType(uint64_t id, SMNonDestructiveClipType& type) override;

        virtual void SetClipOnOrOff(uint64_t id, bool isActive) override;

        virtual void GetAllPolys(bvector<bvector<DPoint3d>>& polys) override;

        virtual void SetAutoCommit(bool autoCommit) override;

        virtual void GetAllCoverageIDs(bvector<uint64_t>& allIds) override;

        virtual void StoreClipWithParameters(const bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        
        virtual void LoadClipWithParameters(bvector<DPoint3d>& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive) override;

        virtual void StoreClipWithParameters(const ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;

        virtual void LoadClipWithParameters(ClipVectorPtr& clipData, uint64_t id, SMClipGeometryType& geom, SMNonDestructiveClipType& type, bool& isActive) override;

};

class SMSQLiteLinearFeaturesExtOps : public ILinearFeaturesExtOps
    {
    private:

        SMSQLiteFilePtr            m_smSQLiteFile;


    public:

        SMSQLiteLinearFeaturesExtOps(SMSQLiteFilePtr& smSQLiteFile);

        virtual ~SMSQLiteLinearFeaturesExtOps();

        virtual uint64_t StoreFeature(uint32_t type, const bvector<DPoint3d>& featureData) override;

        virtual void GetFeature(uint64_t id, uint32_t& type, bvector<DPoint3d>& featureData) override;

    };

