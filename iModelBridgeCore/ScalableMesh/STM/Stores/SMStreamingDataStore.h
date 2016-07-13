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


#if 0 

template <class POINT, class EXTENT> class SMStreamingNodeDataStore : public ISMNodeDataStore<POINT> 
    {
    private:
        
        SMIndexNodeHeader<EXTENT>*    m_nodeHeader;
        DataSourceAccount*            m_dataSourceAccount;
        WString                       m_pathToPoints;
        WString                       m_storage_connection_string;
        scalable_mesh::azure::Storage m_stream_store;

        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        mutable std::map<ISMStore::NodeID, PointBlock> m_pointCache;
        mutable std::mutex m_pointCacheLock;

    public:

        enum SMStreamingDataType
            {
            POINTS,
            INDICES,
            UVS,
            UVINDICES
            };
              
        SMStreamingNodePointStore(DataSourceAccount *dataSourceAccount, const WString& path, SMStreamingDataType type, SMIndexNodeHeader<EXTENT>* nodeHeader, bool compress = true);
            
        virtual ~SMStreamingNodePointStore();
              
        virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData) override;
            
        virtual HPMBlockID StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;
            
        virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        virtual bool DestroyBlock(HPMBlockID blockID) override;         

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;        
    };

#endif