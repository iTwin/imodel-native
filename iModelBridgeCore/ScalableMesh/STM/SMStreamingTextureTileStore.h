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

class StreamingTextureTileStore : public IScalableMeshDataStore<uint8_t, float, float>
    {
    public:

        // Helper texture data structure
        struct Texture : public bvector<uint8_t>
            {
            public:
                Texture() : m_stream_store(nullptr) {}

                Texture(const int& width, const int& height, const int& numChannels, scalable_mesh::azure::Storage* store = nullptr)
                    : m_Width{width}, m_Height{height}, m_NbChannels(numChannels), m_stream_store(store)
                    {}
                void SetDataSource(const WString& pi_DataSource)
                    {
                    m_DataSource = pi_DataSource;
                    }
                void SetStore(const scalable_mesh::azure::Storage& pi_Store)
                    {
                    m_stream_store = &pi_Store;
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
                void Load()
                    {
                    assert(m_ID != -1);
                    wstringstream ss;
                    ss << m_DataSource + L"t_" << m_ID << L".bin";
                    auto filename = ss.str();

                    if (s_stream_from_disk)
                        {
                        this->LoadFromLocal(filename);
                        }
                    else if (s_stream_from_file_server)
                        {
                        this->LoadFromFileSystem(filename);
                        }
                    else {
                        this->LoadFromAzure(filename);
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
                        std::wcout << U("Error: ") << e.what() << std::endl;
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
                void LoadFromLocal(const wstring& m_pFilename)
                    {
                    BeFile file;
                    auto fileOpenStatus = OPEN_FILE(file, m_pFilename.c_str(), BeFileAccess::Read);
                    if (BeFileStatus::Success != fileOpenStatus) return;

                    assert(fileOpenStatus == BeFileStatus::Success);

                    bvector<uint8_t> entire_file;
                    BeFileStatus read_result = file.ReadEntireFile(entire_file);
                    assert(BeFileStatus::Success == read_result);

                    // Read texture header
                    memcpy(&m_Width, entire_file.data(), sizeof(int));
                    memcpy(&m_Height, entire_file.data() + sizeof(int), sizeof(int));
                    memcpy(&m_NbChannels, entire_file.data() + 2 * sizeof(int), sizeof(int));
                    memcpy(&m_Format, entire_file.data() + 3 * sizeof(int), sizeof(int));

                    auto textureSize = (uint32_t)(m_Width*m_Height*m_NbChannels);
                    uint64_t compressedSize = entire_file.size() - 4 * sizeof(int);

                    this->DecompressTexture(entire_file.data() + 4 * sizeof(int), compressedSize, textureSize);

                    file.Close();
                    }
                void LoadFromAzure(const wstring& m_pFilename)
                    {
                    assert(m_stream_store != nullptr);
                    bool blobDownloaded = false;
                    m_stream_store->DownloadBlob(m_pFilename.c_str(), [this, &blobDownloaded](scalable_mesh::azure::Storage::point_buffer_type& buffer)
                        {
                        assert(!buffer.empty());

                        // Read texture header
                        m_Width = reinterpret_cast<int&>(buffer[0]);
                        m_Height = reinterpret_cast<int&>(buffer[sizeof(int)]);
                        m_NbChannels = reinterpret_cast<int&>(buffer[2 * sizeof(int)]);
                        m_Format = reinterpret_cast<int&>(buffer[3 * sizeof(int)]);

                        auto textureSize = (uint32_t)(m_Width*m_Height*m_NbChannels);
                        uint32_t compressedSize = (uint32_t)buffer.size() - sizeof(4 * sizeof(int));

                        this->DecompressTexture(&buffer[0] + 4 * sizeof(int), compressedSize, textureSize);
                        blobDownloaded = true;
                        });
                    assert(blobDownloaded);

                    }
                void LoadFromFileSystem(const wstring& m_pFilename)
                    {
                    assert(false); // Not implemented yet
                    /*bvector<uint8_t> buffer;
                    DownloadBlockFromFileServer(filename, &buffer, 1000000);
                    assert(!buffer.empty() && buffer.size() <= 1000000);

                    uint32_t UncompressedSize = reinterpret_cast<uint32_t&>(buffer[0]);
                    uint32_t sizeData = (uint32_t)buffer.size();

                    HCDPacket uncompressedPacket, compressedPacket;
                    compressedPacket.SetBuffer(&buffer[0] + sizeof(uint32_t), sizeData - sizeof(uint32_t));
                    compressedPacket.SetDataSize(sizeData - sizeof(uint32_t));
                    uncompressedPacket.SetDataSize(UncompressedSize);
                    LoadCompressedPacket(compressedPacket, uncompressedPacket);
                    assert(UncompressedSize == uncompressedPacket.GetDataSize());
                    points.resize(UncompressedSize);
                    memcpy(points.data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());*/

                    }

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
                const scalable_mesh::azure::Storage* m_stream_store;
            };

        static void OpenOrCreateBeFile(BeFile& file, const WString& path, HPMBlockID blockID)
            {
            wstringstream ss;
            ss << path + L"t_" << blockID.m_integerID << L".bin";
            auto filename = ss.str();
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
                    texture.SetStore(m_stream_store);
                    texture.SetID(blockID.m_integerID);
                    texture.Load();
                    }
                }
            assert(texture.IsLoaded() && !texture.empty());
            return texture;
            }

        StreamingTextureTileStore(const WString& directory)
            : m_path(directory),
            m_stream_store(L"DefaultEndpointsProtocol=https;AccountName=pcdsustest;AccountKey=3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ=="
                           , L"scalablemeshtest")
            {
            m_path += L"textures/";
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

        virtual size_t StoreHeader(float* header, HPMBlockID blockID)
            {
            assert(!"Should not pass here.");
            return 0;
            }

        virtual size_t LoadHeader(float* header, HPMBlockID blockID)
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
            memmove(DataTypeArray + 3 * sizeof(int), texture.data(), std::min(texture.size(), maxCountData));
            texture.Unload();
            return std::min(textureSize + 3 * sizeof(int), maxCountData);
            }

    private:
        WString m_path;
        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        mutable map<uint32_t, Texture> m_textureCache;
        mutable std::mutex m_textureCacheLock;
        // NEEDS_WORK_SM_STREAMING: should only have one stream store for all data types
        scalable_mesh::azure::Storage m_stream_store;
    };
