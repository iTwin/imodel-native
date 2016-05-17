#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
/*#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>*/
//#include <Mtg/MtgStructs.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HRFBmpFile.h>
//#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>

class StreamingTextureTileStore : public IScalableMeshDataStore<uint8_t, float, float> // JPEGData (uint8_t*), size
    {
    public:

        struct Texture : public bvector<uint8_t>
            {
            public:
                Texture()
                    : m_IsLoaded(false),
                      m_IsLoading(false),
                      m_stream_store(nullptr)
                    {}

                void SetDataSource(const WString& pi_DataSource)
                    {
                    m_DataSource = pi_DataSource;
                    }

                void SetStore(const scalable_mesh::azure::Storage& pi_Store)
                    {
                    m_stream_store = &pi_Store;
                    }

                void Load(uint32_t m_pID)
                    {
                    wstringstream ss;
                    ss << m_DataSource << L"t_" << m_pID << L".bin";
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

                size_t GetWidth() { return m_Width; }
                size_t GetHeight() { return m_Height; }

            private:

                void DecompressTexture(uint8_t* pi_CompressedTextureData, uint32_t pi_CompressedTextureSize, uint32_t pi_TextureSize, size_t pi_nOfChannels = 3)
                    {
                    HCDPacket uncompressedPacket, compressedPacket(pi_CompressedTextureData + 4 * sizeof(int), pi_CompressedTextureSize - 4 * sizeof(int), pi_CompressedTextureSize);
                    uncompressedPacket.SetDataSize(pi_TextureSize);

                    m_Width = m_Height = (size_t)sqrt(pi_TextureSize / pi_nOfChannels);
                    auto codec = new HCDCodecIJG(m_Width, m_Height, pi_nOfChannels * 8);// 24 bits per pixels
                    codec->SetQuality(70);
                    codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
                    HFCPtr<HCDCodec> pCodec = codec;
                    uncompressedPacket.SetBufferOwnership(true);
                    uncompressedPacket.SetBuffer(new uint8_t[uncompressedPacket.GetDataSize()], uncompressedPacket.GetDataSize() * sizeof(uint8_t));
                    try {
                        const size_t uncompressedDataSize = pCodec->DecompressSubset(compressedPacket.GetBufferAddress(), compressedPacket.GetDataSize() * sizeof(uint8_t), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetBufferSize() * sizeof(uint8_t));
                        uncompressedPacket.SetDataSize(uncompressedDataSize);
                        assert(pi_TextureSize >= uncompressedDataSize); // JPEG can be lossy

                        this->resize(uncompressedDataSize);
                        memcpy(this->data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());
                        }
                    catch (const std::exception& e)
                        {
                        assert(!"There is an error decompressing texture");
                        std::wcout << U("Error: ") << e.what() << std::endl;
                        }
                    }

                void LoadFromLocal(const wstring& m_pFilename)
                    {
                    BeFile file;
                    auto fileOpenStatus = OPEN_FILE(file, m_pFilename.c_str(), BeFileAccess::Read);
                    if (BeFileStatus::Success != fileOpenStatus) return;

                    assert(fileOpenStatus == BeFileStatus::Success);

                    // Read Uncompressed texture size
                    size_t textureSize = 0;
                    uint32_t bytesRead = 0;
                    auto read_result = file.Read(&textureSize, &bytesRead, sizeof(textureSize));
                    assert(BeFileStatus::Success == read_result);
                    assert(bytesRead == sizeof(textureSize));

                    // Read compressed texture
                    auto DataTypeArray = new uint8_t[textureSize];
                    read_result = file.Read((uint8_t*)DataTypeArray, &bytesRead, (uint32_t)textureSize);
                    assert(BeFileStatus::Success == read_result);
                    assert(bytesRead <= textureSize);

                    uint64_t compressedSize = bytesRead;

                    this->DecompressTexture(DataTypeArray, compressedSize, (uint32_t)textureSize);

                    file.Close();
                    }

                void LoadFromAzure(const wstring& m_pFilename)
                    {
                    assert(m_stream_store != nullptr);
                    bool blobDownloaded = false;
                    m_stream_store->DownloadBlob(m_pFilename.c_str(), [this, &blobDownloaded](scalable_mesh::azure::Storage::point_buffer_type& buffer)
                    {
                    assert(!buffer.empty());
                    size_t UncompressedSize = reinterpret_cast<size_t&>(buffer[0]);
                    uint32_t compressedSize = (uint32_t)buffer.size() - sizeof(size_t);

                    auto DataTypeArray = new uint8_t[UncompressedSize];
                    memcpy(DataTypeArray, &buffer[0] + sizeof(size_t), compressedSize);
                    this->DecompressTexture(DataTypeArray, compressedSize, (uint32_t)UncompressedSize);
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
                bool m_IsLoaded;
                bool m_IsLoading;
                size_t m_Width;
                size_t m_Height;
                WString m_DataSource;
                condition_variable m_TextureCV;
                mutex m_TextureMutex;
                const scalable_mesh::azure::Storage* m_stream_store;
            };

        void OpenOrCreateBeFile(BeFile& file, HPMBlockID blockID)
            {
            wstringstream ss;
            ss << m_path << L"t_" << blockID.m_integerID << L".bin";
            auto filename = ss.str();
            auto fileOpenedOrCreated = BeFileStatus::Success == OPEN_FILE(file, filename.c_str(), BeFileAccess::Write)
                                    || BeFileStatus::Success== file.Create(filename.c_str());
            assert(fileOpenedOrCreated);
            }

        Texture& GetTexture(HPMBlockID blockID) const
            {
            Texture& texture = m_textureCache[blockID.m_integerID];
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
                    texture.Load(blockID.m_integerID);
                    }
                }
            assert(texture.IsLoaded());
            return texture;
            }

        StreamingTextureTileStore(WCharCP directory)
            : m_path(directory),
            m_stream_store(L"DefaultEndpointsProtocol=https;AccountName=pcdsustest;AccountKey=3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ=="
                         , L"scalablemeshtest")
            {
            if (s_stream_from_disk)
                {
                // Create base directory structure to store information if not already done
                // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only
                if (0 == CreateDirectoryW(m_path.c_str(), NULL))
                    {
                    assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }
                }
            else
                {
                // stream from azure
                }
            }

        virtual ~StreamingTextureTileStore()
            {
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

        bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
                                   HCDPacket& pi_compressedPacket, int width, int height, int nOfChannels = 3)
            {
            HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

            // initialize codec
            auto codec = new HCDCodecIJG(width, height, 8 * nOfChannels);
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

        // New interface

        virtual HPMBlockID StoreNewBlock(uint8_t* DataTypeArray, size_t countData)
            {
            assert(!"Should call StoreBlock() instead");
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData);
            pi_uncompressedPacket.SetDataSize(countData);
            size_t w, h;
            w = h = (size_t)sqrt(countData / 3);
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, (int)w, (int)h, 3);
            bvector<uint8_t> texData(pi_compressedPacket.GetDataSize());
            memcpy(&texData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            int64_t id = SQLiteNodeHeader::NO_NODEID;
            //m_smSQLiteFile->StoreIndices(id, texData, countData);
            return HPMBlockID(id);
            }

        virtual HPMBlockID StoreBlock(uint8_t* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            assert(blockID.IsValid());
            //if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData);
            pi_uncompressedPacket.SetDataSize(countData);
            size_t w, h;
            w = h = (size_t)sqrt(countData / 3);
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, (int)w, (int)h, 3);
            bvector<uint8_t> texData(pi_compressedPacket.GetDataSize() + sizeof(size_t));
            reinterpret_cast<size_t&>(*texData.data()) = countData;
            memcpy(&texData[0] + sizeof(size_t), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            BeFile file;
            OpenOrCreateBeFile(file, blockID);
            file.Write(NULL, texData.data(), (uint32_t)(texData.size()));
            file.Close();
            return HPMBlockID(blockID.m_integerID);
            }

        virtual HPMBlockID StoreCompressedBlock(uint8_t* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            assert(blockID.IsValid());
            BeFile file;
            OpenOrCreateBeFile(file, blockID);
            file.Write(NULL, DataTypeArray, (uint32_t)countData);
            file.Close();
            return HPMBlockID(blockID.m_integerID);
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            return this->GetTexture(blockID).size();

            }


        virtual size_t StoreHeader(float* header, HPMBlockID blockID)
            {
            return 0;
            }

        virtual size_t LoadHeader(float* header, HPMBlockID blockID)
            {

            return 0;
            }

        virtual size_t LoadBlock(uint8_t* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            auto& texture = this->GetTexture(blockID);
            /*BeFile file;
            wstringstream ss;
            ss << L"file://"<<m_path << L"textureAfterLoad_" << blockID.m_integerID << L".jpeg";
            auto filename = ss.str();
            HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(filename.c_str()));
            byte* pixelBuffer = new byte[1024 * 1024 * 3];
            size_t t = 0;
            for (size_t i = 0; i < 1024 * 1024 * 4; i += 4)
                {
                pixelBuffer[t] = *(texture.data() + i);
                pixelBuffer[t + 1] = *(texture.data() + i + 1);
                pixelBuffer[t + 2] = *(texture.data() + i + 2);
                t += 3;
                }
            HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
            HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                                      1024,
                                                      1024,
                                                      pImageDataPixelType,
                                                      pixelBuffer);
            delete[] pixelBuffer;*/
            ((int*)DataTypeArray)[0] = (int)texture.GetWidth();
            ((int*)DataTypeArray)[1] = (int)texture.GetHeight();
            ((int*)DataTypeArray)[2] = 3;
            memcpy(DataTypeArray + 3 * sizeof(int), texture.data(), std::min(texture.size(), maxCountData));
            //return texture.size() + 3 * sizeof(int);
            return std::min(texture.size() + 3 * sizeof(int), maxCountData);
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return false;
            }

    private:
        WString m_path;
        mutable map<uint32_t, Texture> m_textureCache;
        // NEEDS_WORK_SM_STREAMING: should only have one stream store for all data types
        scalable_mesh::azure::Storage m_stream_store;
    };
