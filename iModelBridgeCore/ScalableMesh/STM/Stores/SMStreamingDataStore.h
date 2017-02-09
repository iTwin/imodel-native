//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMStreamingDataStore.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include "ISMDataStore.h"
#include "..\SMNodeGroup.h"
#include "SMStoreUtils.h"
#include "SMSQLiteSisterFile.h"
#include <json/json.h>
#include <ImagePP/all/h/HCDCodecIJG.h>

extern bool s_stream_from_disk;
extern bool s_stream_from_wsg;
extern bool s_stream_using_cesium_3d_tiles_format;
extern bool s_stream_using_curl;
extern bool s_stream_from_grouped_store;
extern bool s_stream_enable_caching;
extern bool s_is_virtual_grouping;
extern bool s_is_legacy_dataset;
extern bool s_is_legacy_master_header;
extern bool s_use_azure_sandbox;
extern bool s_use_public_rds;
extern bool s_use_qa_azure;

//extern std::mutex fileMutex;

#ifndef NDEBUG
#define DEBUG_STREAMING_DATA_STORE
extern std::mutex s_consoleMutex;
#endif

class DataSourceAccount; 

template <class EXTENT> class SMStreamingStore : public ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>, public SMSQLiteSisterFile
    {
    public:
        enum FormatType
            {
            Binary,
            Json,
            Cesium3DTiles
            };
    private : 
        
        bool m_use_node_header_grouping = false;
        bool m_use_virtual_grouping = false;
        FormatType m_formatType = FormatType::Binary;
        DataSourceAccount* m_dataSourceAccount;
        WString m_rootDirectory;        
        DataSourceURL m_pathToHeaders;
        SMNodeDistributor<SMNodeGroup::DistributeData>::Ptr m_NodeHeaderFetchDistributor;
        bvector<SMNodeGroup::Ptr> m_nodeHeaderGroups;

    protected : 


        SMNodeGroup::Ptr FindGroup(HPMBlockID blockID);
            
        SMNodeGroup::Ptr GetGroup(HPMBlockID blockID);
            
        void ReadNodeHeaderFromBinary(SMIndexNodeHeader<EXTENT>* header, uint8_t* headerData, uint64_t& maxCountData) const;
        void GetNodeHeaderBinary(const HPMBlockID& blockID, std::unique_ptr<uint8_t>& po_pBinaryData, uint64_t& po_pDataSize);

        void ReadNodeHeaderFromJSON(SMIndexNodeHeader<EXTENT>* header, const Json::Value& nodeHeader) const;

    private :

        DataSourceStatus InitializeDataSourceAccount(DataSourceManager& dataSourceManager, const WString& directory);
        
    public : 
    
        SMStreamingStore(DataSourceManager& dataSourceManager, const WString& path, bool compress = true, bool areNodeHeadersGrouped = false, bool isVirtualGrouping = false, WString headers_path = L"", FormatType formatType = FormatType::Binary);
       
        virtual ~SMStreamingStore();

#ifdef VANCOUVER_API
        static SMStreamingStore* Create(DataSourceManager& dataSourceManager, const WString& path, bool compress = true, bool areNodeHeadersGrouped = false, bool isVirtualGrouping = false, WString headers_path = L"", FormatType formatType = FormatType::Binary)
            {
            return new SMStreamingStore(dataSourceManager, path, compress, areNodeHeadersGrouped, isVirtualGrouping, headers_path, formatType);
            }
#endif

        DataSource *InitializeDataSource(std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize) const;

        DataSourceAccount *GetDataSourceAccount(void) const;

        void SetDataSourceAccount(DataSourceAccount *dataSourceAccount);

        void SetDataFormatType(FormatType formatType);

        static void SerializeHeaderToBinary(const SMIndexNodeHeader<EXTENT>* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize);

        void SerializeHeaderToCesium3DTile(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize) const;

        static void SerializeHeaderToCesium3DTileJSON(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& tile);

        static void SerializeHeaderToJSON(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& block);
                   
        //Inherited from ISMDataStore
        virtual uint64_t GetNextID() const override;
            
        virtual void Close() override;
            
        virtual bool StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;
            
        virtual size_t LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;            

        virtual bool SetProjectFilesPath(BeFileName& projectFilesPath, bool inCreation) override;

        virtual bool GetNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;
                
        virtual bool GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;

        virtual bool GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;        

        virtual bool GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType = SMStoreDataType::Texture) override;

        virtual bool GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType = SMStoreDataType::UvCoords) override;

        


        //Multi-items loading store
        virtual bool GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMTileMeshDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMCesium3DTilesDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        //Inherited from ISMDataStore - End
                             
    };



// Helper point block data structure
struct StreamingDataBlock : public bvector<uint8_t>
    {

    public:

        bool IsLoading();

        bool IsLoaded();

        void LockAndWait();

        void SetLoading();

        DataSource *initializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize);

        void Load(DataSourceAccount *dataSourceAccount, SMStoreDataType dataType, uint64_t dataSize = uint64_t(-1));

        void UnLoad();

        void SetLoaded();

        void SetID(const uint64_t& pi_ID);

        uint64_t GetID();

        void SetDataSourceURL(const DataSourceURL& pi_DataSource);

        void SetDataSourcePrefix(const std::wstring& prefix);

        void SetDataSourceExtension(const std::wstring& extension);

        void DecompressPoints(uint8_t* pi_CompressedData, uint32_t pi_CompressedDataSize, uint32_t pi_UncompressedDataSize);

        DPoint3d* GetPoints();
        int32_t*  GetIndices();
        DPoint2d* GetUVs();
        Byte*     GetTexture();

        uint32_t GetNumberOfPoints();
        uint32_t GetNumberOfIndices();
        uint32_t GetNumberOfUvs();
        uint32_t GetTextureSize();

    protected:

        DataSource::DataSize LoadDataBlock(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]>& destination, uint64_t dataSizeKnown);

    protected:

        bool m_pIsLoading = false;
        bool m_pIsLoaded = false;
        uint64_t m_pID = -1;
        DataSourceURL m_pDataSourceURL;
        std::wstring m_pPrefix = L"p_";
        std::wstring m_extension = L".bin";
        condition_variable m_pDataBlockCV;
        mutex m_pDataBlockMutex;

    private:
        struct Cesium3DTilesData : Cesium3DTilesBase
            {
            uint32_t numPoints;
            uint32_t numIndices;
            uint32_t numUvs;
            uint32_t textureSize;
            uint32_t pointOffset;
            uint32_t indiceOffset;
            uint32_t uvOffset;
            uint32_t textureOffset;
            };
        Cesium3DTilesData m_tileData;

        void ParseCesium3DTilesData(const Byte*, const size_t&);
    };

template <class DATATYPE, class EXTENT> class SMStreamingNodeDataStore : public ISMNodeDataStore<DATATYPE> 
    {        

    public:

        SMStreamingNodeDataStore(DataSourceAccount *dataSourceAccount, SMStoreDataType type, SMIndexNodeHeader<EXTENT>* nodeHeader, SMNodeGroup::Ptr nodeGroup = nullptr, bool compress = true);

        virtual ~SMStreamingNodeDataStore();

        virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;

        virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const override;

        virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;

        virtual bool DestroyBlock(HPMBlockID blockID) override;

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) override;

    protected:

        SMIndexNodeHeader<EXTENT>*    m_nodeHeader;
        DataSourceAccount*            m_dataSourceAccount;
        DataSourceURL                 m_dataSourceURL;

        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        typedef std::map<ISMStore::NodeID, std::unique_ptr<StreamingDataBlock>> DataCache;
        mutable DataCache m_dataCache;
        mutable std::mutex m_dataCacheMutex;

        StreamingDataBlock &   GetBlock(HPMBlockID blockID) const;

    private:
        
        SMNodeGroup::Ptr           m_nodeGroup;
        SMStoreDataType               m_dataType;

        uint64_t GetBlockSizeFromNodeHeader() const;
    };


struct StreamingTextureBlock : public StreamingDataBlock
    {

    public:

        StreamingTextureBlock();

        StreamingTextureBlock(const int& width, const int& height, const int& numChannels);

        void Load(DataSourceAccount *dataSourceAccount, uint64_t blockSizeKnown = uint64_t(-1));

        void Store(DataSourceAccount *dataSourceAccount, uint8_t* DataTypeArray, size_t countData, const HPMBlockID& blockID);

        size_t GetWidth() { return m_Width; }
        size_t GetHeight() { return m_Height; }
        size_t GetNbChannels() { return m_NbChannels; }

    private:

        void DecompressTexture(uint8_t* pi_CompressedTextureData, uint32_t pi_CompressedTextureSize, uint32_t pi_TextureSize);
        bool CompressTexture(const HCDPacket& pi_uncompressedPacket, HCDPacket& pi_compressedPacket);

    private:

        int m_Width = 256;
        int m_Height = 256;
        int m_NbChannels = 3; // 3 channels by default
        int m_Format = 0;     // could be useful in the future
    };
    
template <class DATATYPE, class EXTENT> class StreamingNodeTextureStore : public SMStreamingNodeDataStore<DATATYPE, EXTENT>
    {

    public:

        StreamingTextureBlock& GetTexture(HPMBlockID blockID) const;
            
        StreamingNodeTextureStore(DataSourceAccount *dataSourceAccount, SMIndexNodeHeader<EXTENT>* nodeHeader);

        virtual bool DestroyBlock(HPMBlockID blockID) override;            
                        
        virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;        
            
        virtual HPMBlockID StoreCompressedBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;

        virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const override;

        //Valid only for store handling only one type of data.
        virtual void   ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;

        //Valid only for store handling only multiple type of data.
        virtual void   ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) override;
                            
        virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        void                SetDataSourceAccount    (DataSourceAccount *dataSourceAccount);
        DataSourceAccount * GetDataSourceAccount    (void) const;

    private:

        typedef SMStreamingNodeDataStore<DATATYPE, EXTENT> Super;
    };
