//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMStreamingTextureTileStore.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
#include <ImagePP/all/h/HCDCodecIJG.h>

#include <CloudDataSource/DataSourceAccount.h>

class StreamingTextureTileStore : public IScalableMeshDataStore<uint8_t, float, float>
    {
    public:

        // Helper texture data structure
        struct Texture : public bvector<uint8_t>
            {
            public:
                Texture(){}

                Texture(const int& width, const int& height, const int& numChannels)
                    : m_Width{width}, m_Height{height}, m_NbChannels(numChannels)
                    {}
                void SetDataSource(const WString& pi_DataSource)
                    {
                    m_DataSource = pi_DataSource;
                    }
                void SavePixelDataToDisk(uint8_t* DataTypeArray, size_t countData, const HPMBlockID& blockID)
                    {
                    // First, compress the texture
                    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
                    pi_uncompressedPacket.SetBuffer(DataTypeArray, countData);
                    pi_uncompressedPacket.SetDataSize(countData);

                    CompressTexture(pi_uncompressedPacket, pi_compressedPacket);

                    // Second, save to disk
                    int format = 0; // Keep an int to define the format and possible other options

                    bvector<uint8_t> texData(4 * sizeof(int) + pi_compressedPacket.GetDataSize());
                    int *pHeader = (int*)(texData.data());
                    pHeader[0] = m_Width;
                    pHeader[1] = m_Height;
                    pHeader[2] = m_NbChannels;
                    pHeader[3] = format;
                    memcpy(texData.data() + 4 * sizeof(int), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());

                    BeFile file;
                    StreamingTextureTileStore::OpenOrCreateBeFile(file, m_DataSource, blockID);
                    file.Write(NULL, texData.data(), (uint32_t)(texData.size()));
                    file.Close();
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

                void Load(DataSourceAccount *dataSourceAccount)
                {
                    std::unique_ptr<DataSource::Buffer[]>       dest;
                    DataSource                              *   dataSource;
                    DataSource::DataSize                        readSize;

                    assert(m_ID != -1);
                    wchar_t buffer[10000];
                    swprintf(buffer, L"%st_%llu.bin", m_DataSource.c_str(), m_ID);

                    DataSourceURL   dataSourceURL(buffer);

                    DataSourceBuffer::BufferSize    destSize = 5 * 1024 * 1024;

                    dataSource = this->InitializeDataSource(dataSourceAccount, dest, destSize);
                    if (dataSource == nullptr)
                        return;

                    if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                        return;

                    if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
                        return;

                    dataSource->close();
//                    dataSourceAccount->destroyDataSource(dataSource);

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

        static void OpenOrCreateBeFile(BeFile& file, const WString& path, HPMBlockID blockID)
            {
            wchar_t buffer[10000];
            swprintf(buffer, L"%st_%llu.bin", path.c_str(), blockID.m_integerID);
            std::wstring filename(buffer);
            auto fileOpenedOrCreated = BeFileStatus::Success == OPEN_FILE(file, filename.c_str(), BeFileAccess::Write)
                || BeFileStatus::Success == file.Create(filename.c_str());
            assert(fileOpenedOrCreated);
            }

        Texture& GetTexture(HPMBlockID blockID) const
            {
            // std::map [] operator is not thread safe while inserting new elements
            m_textureCacheLock.lock();
            Texture& texture = m_textureCache[blockID.m_integerID];
            m_textureCacheLock.unlock();
            assert((texture.GetID() != uint64_t(-1) ? texture.GetID() == blockID.m_integerID : true));
            if (!texture.IsLoaded())
                {
                if (texture.IsLoading())
                    {
                    texture.LockAndWait();
                    }
                else
                    {
                    texture.SetDataSource(m_path);
                    texture.SetID(blockID.m_integerID);
                    texture.Load(this->GetDataSourceAccount());
                    }
                }
            assert(texture.IsLoaded() && !texture.empty());
            return texture;
            }

        StreamingTextureTileStore(DataSourceAccount *dataSourceAccount, const WString& directory)
            : m_path(directory)
            {
            m_path += L"textures/";
            
            this->SetDataSourceAccount(dataSourceAccount);
            
            // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
            if (s_stream_from_disk)
                {
                // Create base directory structure to store information if not already done
                // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only
                if (0 == CreateDirectoryW(m_path.c_str(), NULL))
                    {
                    assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }
                }
            }


        virtual ~StreamingTextureTileStore()
            {
            for (auto it = m_textureCache.begin(); it != m_textureCache.end(); ++it) it->second.clear();
            m_textureCache.clear();
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return false;
            }

        virtual void Close()
            {
            }

        virtual bool StoreMasterHeader(float* indexHeader, size_t headerSize)
            {
            return false;
            }

        virtual size_t LoadMasterHeader(float* indexHeader, size_t headerSize)
            {
            return 0;
            }

        virtual HPMBlockID StoreNewBlock(uint8_t* DataTypeArray, size_t countData)
            {
            assert(!"Should call StoreBlock() instead");
            int64_t id = SQLiteNodeHeader::NO_NODEID;
            return HPMBlockID(id);
            }

        virtual HPMBlockID StoreBlock(uint8_t* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            assert(blockID.IsValid());

            // The data block starts with 12 bytes of metadata (texture header), followed by pixel data
            Texture texture(((int*)DataTypeArray)[0], ((int*)DataTypeArray)[1], ((int*)DataTypeArray)[2]);
            texture.SetDataSource(m_path);
            texture.SavePixelDataToDisk(DataTypeArray + 3 * sizeof(int), countData - 3 * sizeof(int), blockID);

            return blockID;
            }

        virtual HPMBlockID StoreCompressedBlock(uint8_t* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            assert(blockID.IsValid());
            BeFile file;
            OpenOrCreateBeFile(file, m_path, blockID);
            file.Write(NULL, DataTypeArray, (uint32_t)countData);
            file.Close();
            return HPMBlockID(blockID.m_integerID);
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            return this->GetTexture(blockID).size() + 3 * sizeof(int);
            }

        virtual size_t StoreNodeHeader(float* header, HPMBlockID blockID)
            {
            assert(!"Should not pass here.");
            return 0;
            }

        virtual size_t LoadNodeHeader(float* header, HPMBlockID blockID)
            {
            assert(!"Should not pass here.");
            return 0;
            }

        virtual size_t LoadBlock(uint8_t* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            auto& texture = this->GetTexture(blockID);
            auto textureSize = texture.size();
            assert(textureSize + 3 * sizeof(int) == maxCountData);
            ((int*)DataTypeArray)[0] = (int)texture.GetWidth();
            ((int*)DataTypeArray)[1] = (int)texture.GetHeight();
            ((int*)DataTypeArray)[2] = (int)texture.GetNbChannels();
            assert(maxCountData >= texture.size());
            memmove(DataTypeArray + 3 * sizeof(int), texture.data(), std::min(texture.size(), maxCountData));
            m_textureCacheLock.lock();
            texture.Unload();
            m_textureCache.erase(texture.GetID());
            m_textureCacheLock.unlock();
            return std::min(textureSize + 3 * sizeof(int), maxCountData);
            }

        void                SetDataSourceAccount    (DataSourceAccount *dataSourceAccount)  {m_dataSourceAccount = dataSourceAccount;}
        DataSourceAccount * GetDataSourceAccount    (void) const                            {return m_dataSourceAccount;}

    private:
        WString m_path;
        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        mutable map<uint32_t, Texture> m_textureCache;
        mutable std::mutex m_textureCacheLock;

        DataSourceAccount * m_dataSourceAccount;
    };
