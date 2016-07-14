//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMStreamingDataStore.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include "ISMDataStore.h"
#include "..\SMNodeGroup.h"
#include "SMStoreUtils.h"

class DataSourceAccount; 

template <class EXTENT> class SMStreamingStore : public ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>
    {
    private : 
        
        DataSourceAccount* m_dataSourceAccount;
        WString m_rootDirectory;        
        WString m_pathToHeaders;
        bool m_use_node_header_grouping;
        // NEEDS_WORK_SM_STREAMING: should only have one stream store for all data types
        WString m_storage_connection_string;
        scalable_mesh::azure::Storage m_stream_store;
        SMNodeDistributor<SMNodeGroup::DistributeData>::Ptr m_NodeHeaderFetchDistributor;
        bvector<HFCPtr<SMNodeGroup>> m_nodeHeaderGroups;

    protected : 


        HFCPtr<SMNodeGroup> FindGroup(HPMBlockID blockID);
            
        HFCPtr<SMNodeGroup> GetGroup(HPMBlockID blockID);
            
        void ReadNodeHeaderFromBinary(SMIndexNodeHeader<EXTENT>* header, uint8_t* headerData, uint64_t& maxCountData) const;
            
        void GetNodeHeaderBinary(const HPMBlockID& blockID, std::unique_ptr<uint8_t>& po_pBinaryData, uint64_t& po_pDataSize);
        
    public : 
    
        SMStreamingStore(DataSourceAccount *dataSourceAccount, const WString& path, bool compress = true, bool areNodeHeadersGrouped = false, WString headers_path = L"");
       
        virtual ~SMStreamingStore();
                   
        //Inherited from ISMDataStore
        virtual uint64_t GetNextID() const override;
            
        virtual void Close() override;
            
        virtual bool StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;
            
        virtual size_t LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;            
                
        virtual bool GetNodeDataStore(ISMPointDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;
        //Inherited from ISMDataStore - End

        DataSource *initializeDataSource(std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize) const;

        DataSourceAccount *getDataSourceAccount(void) const;

        void setDataSourceAccount(DataSourceAccount *dataSourceAccount);        

        void SerializeHeaderToBinary(const SMIndexNodeHeader<EXTENT>* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize) const;
            
        void SerializeHeaderToJSON(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& block);            
            
    };



// Helper point block data structure
struct StreamingDataBlock : public bvector<uint8_t> {
public:
    bool IsLoading();
    bool IsLoaded();

    void LockAndWait();
            
    void SetLoading();

    DataSource *initializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize);
        
    void Load_Old();
                   
    void Load(DataSourceAccount *dataSourceAccount);
        
    void UnLoad();
            
    void SetLoaded();
        
    void SetID(const uint64_t& pi_ID);
        
    uint64_t GetID();

    void SetDataSource(const WString& pi_DataSource);
        
    void SetStore(const scalable_mesh::azure::Storage& pi_Store);
        
    void DecompressPoints(uint8_t* pi_CompressedData, uint32_t pi_CompressedDataSize, uint32_t pi_UncompressedDataSize);
        
private:
    struct MemoryStruct {
        bvector<Byte>* memory;
        size_t         size;
        };

    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
        
    StatusInt LoadFromLocal(const WString& m_pFilename);
        
    StatusInt LoadFromAzure(const WString& m_pFilename);
      
    void LoadFromFileSystem(const WString& m_pFilename);
    
private:

    bool m_pIsLoading = false;
    bool m_pIsLoaded = false;
    uint64_t m_pID = -1;
    WString m_pDataSource;
    const scalable_mesh::azure::Storage* m_stream_store;
    condition_variable m_pPointBlockCV;
    mutex m_pPointBlockMutex;

    };

template <class DATATYPE, class EXTENT> class SMStreamingNodeDataStore : public ISMNodeDataStore<DATATYPE> 
    {
    private:
        
        SMIndexNodeHeader<EXTENT>*    m_nodeHeader;
        DataSourceAccount*            m_dataSourceAccount;
        WString                       m_pathToPoints;
        WString                       m_storage_connection_string;
        scalable_mesh::azure::Storage m_stream_store;

        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        mutable std::map<ISMStore::NodeID, StreamingDataBlock    > m_pointCache;
        mutable std::mutex m_pointCacheLock;

    protected: 

        StreamingDataBlock    & GetBlock(HPMBlockID blockID) const;

    public:

        enum SMStreamingDataType
            {
            POINTS,
            INDICES,
            UVS,
            UVINDICES
            };
              
        SMStreamingNodeDataStore(DataSourceAccount *dataSourceAccount, const WString& path, SMStreamingDataType type, SMIndexNodeHeader<EXTENT>* nodeHeader, bool compress = true);
            
        virtual ~SMStreamingNodeDataStore();
              
        virtual HPMBlockID StoreNewBlock(DATATYPE* DataTypeArray, size_t countData) override;
            
        virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;
            
        virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        virtual bool DestroyBlock(HPMBlockID blockID) override;         

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;        
    };
