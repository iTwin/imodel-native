#pragma once

#include "ISMDataStore.h"

////MST_TS
#include "..\SMSQLiteFile.h"

#include "SMStoreUtils.h"

//typedef ISMDataStore<SMIndexMasterHeader<DRange3d>, SMIndexNodeHeader<DRange3d>> ISMDataStoreType;

template <class EXTENT> class SMSQLiteStore : public ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>
    {
    private : 

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
        
        virtual RefCountedPtr<ISMNodeDataStore<DPoint3d, SMIndexNodeHeader<EXTENT>>> GetNodeDataStore(SMIndexNodeHeader<EXTENT>* nodeHeader) override;
    };


template <class POINT, class EXTENT> class SMSQLiteNodePointStore : public ISMNodeDataStore<POINT, SMIndexNodeHeader<EXTENT>> 
    {
    private:

        SMSQLiteFilePtr m_smSQLiteFile;
    
    public:
              
        SMSQLiteNodePointStore(SMIndexNodeHeader<EXTENT>* nodeHeader,/* ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>* dataStore,*/ SMSQLiteFilePtr& smSQLiteFile);
            
        virtual ~SMSQLiteNodePointStore();
              
        virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData) override;
            
        virtual HPMBlockID StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;
            
        virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        virtual bool DestroyBlock(HPMBlockID blockID) override;                       
    };