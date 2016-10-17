#pragma once

#include "ISMDataStore.h"
#include "SMSQLiteSisterFile.h"
#include "SMStoreUtils.h"

template <class EXTENT> class SMSQLiteStore : public ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>, public SMSQLiteSisterFile
    {        
    private:
        SMSQLiteFilePtr m_smSQLiteFile;

    public : 
    
        SMSQLiteStore(SMSQLiteFilePtr database);
            
        virtual ~SMSQLiteStore();
                    
        virtual uint64_t GetNextID() const override;
            
        virtual void Close() override;
            
        virtual bool StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;
            
        virtual size_t LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;

        virtual bool SetProjectFilesPath(BeFileName& projectFilesPath) override;
                        
        virtual bool GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;

        virtual bool GetNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;        

        virtual bool GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType = SMStoreDataType::Texture) override;

        virtual bool GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;


        //Multi-items loading store
        virtual bool GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        
        static RefCountedPtr<ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>> Create (SMSQLiteFilePtr database)
        {
        return new SMSQLiteStore(database);
        }
        
    };

template <class DATATYPE, class EXTENT> class SMSQLiteNodeDataStore : public ISMNodeDataStore<DATATYPE> 
    {    
    private:

        SMSQLiteFilePtr            m_smSQLiteFile;
        SMIndexNodeHeader<EXTENT>* m_nodeHeader;
        SMStoreDataType            m_dataType;

        void   GetCompressedBlock(bvector<uint8_t>& nodeData, size_t& uncompressedSize, HPMBlockID blockID);

        size_t DecompressTextureData(bvector<uint8_t>& ptData, DATATYPE* DataTypeArray, size_t uncompressedSize);

        HPMBlockID StoreTexture(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID);

    public:      
              
        SMSQLiteNodeDataStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader,/* ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>* dataStore,*/ SMSQLiteFilePtr& smSQLiteFile);
            
        virtual ~SMSQLiteNodeDataStore();                      
            
        virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;

        virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const override;
            
        virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;

        virtual size_t LoadCompressedBlock(bvector<DATATYPE>& DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        virtual bool DestroyBlock(HPMBlockID blockID) override;         

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;        

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) override;

        virtual bool GetClipDefinitionExtOps(IClipDefinitionExtOpsPtr& clipDefinitionExOpsPtr) override;            
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

        virtual void GetAllPolys(bvector<bvector<DPoint3d>>& polys) override;

        virtual void SetAutoCommit(bool autoCommit) override;
    };

