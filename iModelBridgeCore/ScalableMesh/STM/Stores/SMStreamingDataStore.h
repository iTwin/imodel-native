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
#include <json/json.h>
#include <ImagePP/all/h/HCDCodecIJG.h>

extern bool s_stream_from_disk;
extern bool s_stream_from_file_server;
extern bool s_stream_from_wsg;
extern bool s_stream_from_grouped_store;
extern bool s_stream_enable_caching;
extern bool s_is_virtual_grouping;

//extern std::mutex fileMutex;

#ifndef NDEBUG
#define DEBUG_STREAMING_DATA_STORE
extern std::mutex s_consoleMutex;
#endif

class DataSourceAccount; 

template <class EXTENT> class SMStreamingStore : public ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>
    {
    private : 
        
        DataSourceAccount* m_dataSourceAccount;
        WString m_rootDirectory;        
        WString m_pathToHeaders;
        bool m_use_node_header_grouping;
        bool m_use_virtual_grouping;
        SMNodeDistributor<SMNodeGroup::DistributeData>::Ptr m_NodeHeaderFetchDistributor;
        bvector<HFCPtr<SMNodeGroup>> m_nodeHeaderGroups;

    protected : 


        HFCPtr<SMNodeGroup> FindGroup(HPMBlockID blockID);
            
        HFCPtr<SMNodeGroup> GetGroup(HPMBlockID blockID);
            
        void ReadNodeHeaderFromBinary(SMIndexNodeHeader<EXTENT>* header, uint8_t* headerData, uint64_t& maxCountData) const;
            
        void GetNodeHeaderBinary(const HPMBlockID& blockID, std::unique_ptr<uint8_t>& po_pBinaryData, uint64_t& po_pDataSize);
        
    public : 
    
        SMStreamingStore(DataSourceAccount *dataSourceAccount, bool compress = true, bool areNodeHeadersGrouped = false, bool isVirtualGrouping = false, WString headers_path = L"");
       
        virtual ~SMStreamingStore();

        DataSource *InitializeDataSource(std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize) const;

        DataSourceAccount *GetDataSourceAccount(void) const;

        void SetDataSourceAccount(DataSourceAccount *dataSourceAccount);        

        void SerializeHeaderToBinary(const SMIndexNodeHeader<EXTENT>* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize) const;
            
        void SerializeHeaderToJSON(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& block);   
                   
        //Inherited from ISMDataStore
        virtual uint64_t GetNextID() const override;
            
        virtual void Close() override;
            
        virtual bool StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize) override;
            
        virtual size_t StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;
            
        virtual size_t LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID) override;            

        virtual bool SetProjectFilesPath(BeFileName& projectFilesPath) override;

        virtual bool GetNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        virtual bool GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;
                
        virtual bool GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;

        virtual bool GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType) override;        

        virtual bool GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType = SMStoreDataType::Texture) override;

        virtual bool GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        


        //Multi-items loading store
        virtual bool GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader) override;

        static RefCountedPtr<ISMDataStore<SMIndexMasterHeader<EXTENT>, SMIndexNodeHeader<EXTENT>>> Create(DataSourceAccount *dataSourceAccount, bool compress = true, bool areNodeHeadersGrouped = false, bool isVirtualGrouping = false, WString headers_path = L"")
        {
        return new SMStreamingStore(dataSourceAccount, compress, areNodeHeadersGrouped, isVirtualGrouping, headers_path);
        }
        //Inherited from ISMDataStore - End
                             
    };



// Helper point block data structure
struct StreamingDataBlock : public bvector<uint8_t> {
public:
    bool IsLoading();
    bool IsLoaded();

    void LockAndWait();
            
    void SetLoading();

    DataSource *initializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize);
        
    void Load(DataSourceAccount *dataSourceAccount, uint64_t dataSize = uint64_t(-1));
        
    void UnLoad();
            
    void SetLoaded();
        
    void SetID(const uint64_t& pi_ID);
        
    uint64_t GetID();

    void SetDataSource(const WString& pi_DataSource);            
        
    void DecompressPoints(uint8_t* pi_CompressedData, uint32_t pi_CompressedDataSize, uint32_t pi_UncompressedDataSize);
        
private:
    // NEEDS_WORK_SM_STREAMING: Move this to the CloudDataSource?
//    struct MemoryStruct {
//        bvector<Byte>* memory;
//        size_t         size;
//        };
//
// NEEDS_WORK_SM_STREAMING: Move this to the CloudDataSource?
//    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
//
// NEEDS_WORK_SM_STREAMING: Move this to the CloudDataSource?
//    void LoadFromFileSystem(const WString& m_pFilename);

private:

    bool m_pIsLoading = false;
    bool m_pIsLoaded = false;
    uint64_t m_pID = -1;
    WString m_pDataSource;    
    condition_variable m_pPointBlockCV;
    mutex m_pPointBlockMutex;
    };

template <class DATATYPE, class EXTENT> class SMStreamingNodeDataStore : public ISMNodeDataStore<DATATYPE> 
    {        
    private:
        
        SMIndexNodeHeader<EXTENT>*    m_nodeHeader;
        HFCPtr<SMNodeGroup>           m_nodeGroup;
        DataSourceAccount*            m_dataSourceAccount;
        WString                       m_pathToNodeData;
        SMStoreDataType               m_dataType;

        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        mutable std::map<ISMStore::NodeID, StreamingDataBlock    > m_pointCache;
        mutable std::mutex m_pointCacheLock;

        uint64_t GetBlockSizeFromNodeHeader() const;

    protected: 

        StreamingDataBlock    & GetBlock(HPMBlockID blockID) const;

    public:
       
        SMStreamingNodeDataStore(DataSourceAccount *dataSourceAccount, SMStoreDataType type, SMIndexNodeHeader<EXTENT>* nodeHeader, HFCPtr<SMNodeGroup> nodeGroup = nullptr, bool compress = true);
            
        virtual ~SMStreamingNodeDataStore();                      
            
        virtual HPMBlockID StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID) override;
            
        virtual size_t GetBlockDataCount(HPMBlockID blockID) const override;

        virtual size_t GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const override;
                    
        virtual size_t LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID) override;
            
        virtual bool DestroyBlock(HPMBlockID blockID) override;         

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) override;

        virtual void ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) override;
    };


struct Texture : public bvector<uint8_t>
    {
    public:
        Texture(){}

        Texture(const int& width, const int& height, const int& numChannels)
            : m_Width{width}, m_Height{height}, m_NbChannels(numChannels)
            {}
        void SetDataSource(DataSourceAccount * dataSourceAccount, const WString& pi_DataSource)
            {
            m_dataSourceAccount = dataSourceAccount;
            m_DataSource = pi_DataSource;
            }
        void SavePixelDataToDisk(uint8_t* DataTypeArray, size_t countData, const HPMBlockID& blockID)
            {
            // First, compress the texture
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData);
            pi_uncompressedPacket.SetDataSize(countData);

            CompressTexture(pi_uncompressedPacket, pi_compressedPacket);

            // Second, save to DataSource
            int format = 0; // Keep an int to define the format and possible other options

            bvector<uint8_t> texData(4 * sizeof(int) + pi_compressedPacket.GetDataSize());
            int *pHeader = (int*)(texData.data());
            pHeader[0] = m_Width;
            pHeader[1] = m_Height;
            pHeader[2] = m_NbChannels;
            pHeader[3] = format;
            memcpy(texData.data() + 4 * sizeof(int), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());

            DataSourceStatus writeStatus;
            wchar_t buffer[10000];
            swprintf(buffer, L"%st_%llu.bin", m_DataSource.c_str(), blockID.m_integerID);
            DataSourceURL    dataSourceURL(buffer);

            bool created = false;
            DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource(&created);
            assert(dataSource != nullptr);
            //{
            //std::lock_guard<mutex> clk(s_consoleMutex);
            //if (!created) std::cout << "[" << std::this_thread::get_id() << "] A datasource is being reused by thread" << std::endl;
            //else std::cout << "[" << std::this_thread::get_id() << "] New thread DataSource created" << std::endl;
            //}

            writeStatus = dataSource->open(dataSourceURL, DataSourceMode_Write_Segmented);
            assert(writeStatus.isOK()); // problem opening a DataSource

            writeStatus = dataSource->write(texData.data(), (uint32_t)(texData.size()));
            assert(writeStatus.isOK()); // problem writing a DataSource

            writeStatus = dataSource->close();
            assert(writeStatus.isOK()); // problem closing a DataSource
            //{
            //std::lock_guard<mutex> clk(s_consoleMutex);
            //std::cout << "[" << std::this_thread::get_id() << "] Thread DataSource finished" << std::endl;
            //}
            }

        DataSource *InitializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize) const
            {
            if (dataSourceAccount == nullptr)
                return nullptr;
                                                    // Get the thread's DataSource or create a new one
            DataSource *dataSource = dataSourceAccount->getOrCreateThreadDataSource();
            if (dataSource == nullptr)
                return nullptr;
                                                    // Make sure caching is enabled for this DataSource
            dataSource->setCachingEnabled(s_stream_enable_caching);

            dest.reset(new unsigned char[destSize]);
                                                    // Return the DataSource
            return dataSource;
            }

        void Load(uint64_t blockSize = uint64_t(-1))
            {
            std::unique_ptr<DataSource::Buffer[]>       dest;
            DataSource                              *   dataSource;
            DataSource::DataSize                        readSize;

            assert(m_ID != -1);
            wchar_t buffer[10000];
            swprintf(buffer, L"%st_%llu.bin", m_DataSource.c_str(), m_ID);

            DataSourceURL   dataSourceURL(buffer);

            DataSourceBuffer::BufferSize    destSize = 5 * 1024 * 1024;

            dataSource = this->InitializeDataSource(m_dataSourceAccount, dest, destSize);
            if (dataSource == nullptr)
                return;

            if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                return;

            if (blockSize == uint64_t(-1)) blockSize = 0;
            if (dataSource->read(dest.get(), destSize, readSize, blockSize).isFailed())
                return;

            dataSource->close();
            //dataSourceAccount->destroyDataSource(dataSource);

            if (readSize > 0)
                {
                m_Width = reinterpret_cast<int&>(dest.get()[0]);
                m_Height = reinterpret_cast<int&>(dest.get()[sizeof(int)]);
                m_NbChannels = reinterpret_cast<int&>(dest.get()[2 * sizeof(int)]);
                m_Format = reinterpret_cast<int&>(dest.get()[3 * sizeof(int)]);

                auto textureSize = (uint32_t)(m_Width*m_Height*m_NbChannels);
                uint32_t compressedSize = (uint32_t)readSize - sizeof(4 * sizeof(int));

                DecompressTexture(&(dest.get())[0] + 4 * sizeof(int), compressedSize, textureSize);
                }

            m_IsLoaded = true;
            }

        bool IsLoaded() { return m_IsLoaded; }
        bool IsLoading() { return m_IsLoading; }
        void LockAndWait()
            {
            unique_lock<mutex> lock(m_TextureMutex);
            m_TextureCV.wait(lock, [this]() { return m_IsLoaded; });
            }
        void SetLoading()
            {
            m_IsLoaded = false;
            m_IsLoading = true;
            }
        void SetLoaded()
            {
            m_IsLoaded = true;
            m_IsLoading = false;
            m_TextureCV.notify_all();
            }
        void Unload()
            {
            m_IsLoaded = false;
            m_IsLoading = false;
            m_TextureCV.notify_all();
            this->clear();
            }
        void SetID(const uint64_t& pi_ID)
            {
            m_ID = pi_ID;
            }
        uint64_t GetID()
            {
            return m_ID;
            }
        size_t GetWidth() { return m_Width; }
        size_t GetHeight() { return m_Height; }
        size_t GetNbChannels() { return m_NbChannels; }

    private:

        void DecompressTexture(uint8_t* pi_CompressedTextureData, uint32_t pi_CompressedTextureSize, uint32_t pi_TextureSize)
            {
            assert(m_Width > 0 && m_Height > 0 && m_NbChannels > 0);

            auto codec = new HCDCodecIJG(m_Width, m_Height, m_NbChannels * 8);// m_NbChannels * 8 bits per pixels
            codec->SetQuality(70);
            codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
            HFCPtr<HCDCodec> pCodec = codec;
            try {
                this->resize(pi_TextureSize);
                const size_t uncompressedDataSize = pCodec->DecompressSubset(pi_CompressedTextureData, pi_CompressedTextureSize, this->data(), pi_TextureSize);
                assert(pi_TextureSize == uncompressedDataSize);
                }
            catch (const std::exception& e)
                {
                assert(!"There is an error decompressing texture");
                std::wcout << L"Error: " << e.what() << std::endl;
                }
            }

        bool CompressTexture(const HCDPacket& pi_uncompressedPacket, HCDPacket& pi_compressedPacket)
            {
            HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

            // initialize codec
            auto codec = new HCDCodecIJG(m_Width, m_Height, 8 * m_NbChannels);
            codec->SetQuality(70);
            codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
            HFCPtr<HCDCodec> pCodec = codec;
            pi_compressedPacket.SetBufferOwnership(true);
            size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
            pi_compressedPacket.SetBuffer(new uint8_t[compressedBufferSize], compressedBufferSize * sizeof(uint8_t));
            const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(uint8_t), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(uint8_t));
            pi_compressedPacket.SetDataSize(compressedDataSize);

            return true;
            }

        void                SetDataSourceAccount    (DataSourceAccount *dataSourceAccount)      { m_dataSourceAccount = dataSourceAccount; }
        DataSourceAccount * GetDataSourceAccount    (void) const                                { return m_dataSourceAccount; }

    private:
        uint64_t m_ID = -1;
        bool m_IsLoaded = false;
        bool m_IsLoading = false;
        int m_Width = 256;
        int m_Height = 256;
        int m_NbChannels = 3; // 3 channels by default
        int m_Format = 0;     // could be useful in the future
        WString m_DataSource;
        condition_variable m_TextureCV;
        mutex m_TextureMutex;

        DataSourceAccount * m_dataSourceAccount;
    };
    
template <class DATATYPE, class EXTENT> class StreamingNodeTextureStore : public ISMNodeDataStore<DATATYPE> 
    {
    private:

        WString m_path;
        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        mutable map<uint32_t, Texture> m_textureCache;
        mutable std::mutex m_textureCacheLock;

        SMIndexNodeHeader<EXTENT>* m_nodeHeader;
        DataSourceAccount *        m_dataSourceAccount;

    public:              

        Texture& GetTexture(HPMBlockID blockID) const;
            
        StreamingNodeTextureStore(DataSourceAccount *dataSourceAccount, SMIndexNodeHeader<EXTENT>* nodeHeader);

        virtual ~StreamingNodeTextureStore();
            
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
    
    };
