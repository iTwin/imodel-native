#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
/*#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>*/
//#include <Mtg/MtgStructs.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HRFBmpFile.h>
//#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>

#include <CloudDataSource/DataSourceAccount.h>

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


				DataSource *initializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize) const
				{
					if (dataSourceAccount == nullptr)
						return nullptr;
															// Get the thread's DataSource or create a new one
					DataSource *dataSource = dataSourceAccount->getOrCreateThreadDataSource();
					if (dataSource == nullptr)
						return nullptr;
															// Make sure caching is enabled for this DataSource
					dataSource->setCachingEnabled(true);

					dest.reset(new unsigned char[destSize]);
															// Return the DataSource
					return dataSource;
				}

				void Load(DataSourceAccount *dataSourceAccount, uint32_t m_pID)
				{
					std::unique_ptr<DataSource::Buffer[]>		dest;
					DataSource								*	dataSource;
					DataSource::DataSize						readSize;

					wstringstream ss;
					ss << m_DataSource << L"t_" << m_pID << L".bin";

					DataSourceURL	dataSourceURL(ss.str());

					DataSourceBuffer::BufferSize	destSize = 5 * 1024 * 1024;

					dataSource = initializeDataSource(dataSourceAccount, dest, destSize);
					if (dataSource == nullptr)
						return;

					if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
						return;

					if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
						return;

					dataSource->close();

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

                void Load_Old(uint32_t m_pID)
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
                size_t GetNbChannels() { return m_NbChannels; }

            private:

                void DecompressTexture(uint8_t* pi_CompressedTextureData, uint32_t pi_CompressedTextureSize, uint32_t pi_TextureSize, size_t pi_nOfChannels = 3)
                    {
					assert(m_Width > 0 && m_Height > 0 && m_NbChannels > 0);
					
                    HCDPacket uncompressedPacket, compressedPacket(pi_CompressedTextureData, pi_CompressedTextureSize, pi_CompressedTextureSize);
                    uncompressedPacket.SetDataSize(pi_TextureSize);

                    auto codec = new HCDCodecIJG(m_Width, m_Height, m_NbChannels * 8);// m_NbChannels * 8 bits per pixels
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

                    // Read texture header
                    uint32_t bytesRead = 0;
                    BeFileStatus read_result = BeFileStatus::Success;
                    read_result = file.Read(&m_Width, &bytesRead, sizeof(m_Width));
                    assert(BeFileStatus::Success == read_result);
                    assert(bytesRead == sizeof(m_Width));
                    read_result = file.Read(&m_Height, &bytesRead, sizeof(m_Height));
                    assert(BeFileStatus::Success == read_result);
                    assert(bytesRead == sizeof(m_Height));
                    read_result = file.Read(&m_NbChannels, &bytesRead, sizeof(m_NbChannels));
                    assert(BeFileStatus::Success == read_result);
                    assert(bytesRead == sizeof(m_NbChannels));
                    read_result = file.Read(&m_Format, &bytesRead, sizeof(m_Format));
                    assert(BeFileStatus::Success == read_result);
                    assert(bytesRead == sizeof(m_Format));

                    // Read compressed texture
                    auto textureSize = m_Width*m_Height*m_NbChannels;
                    auto DataTypeArray = new uint8_t[textureSize];
                    read_result = file.Read((uint8_t*)DataTypeArray, &bytesRead, (uint32_t)textureSize);
                    assert(BeFileStatus::Success == read_result);
                    assert(bytesRead <= (uint32_t)textureSize);

                    uint64_t compressedSize = bytesRead;

                    this->DecompressTexture(DataTypeArray, compressedSize, (uint32_t)textureSize, m_NbChannels);

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
                    m_NbChannels = reinterpret_cast<int&>(buffer[2*sizeof(int)]);
                    m_Format = reinterpret_cast<int&>(buffer[3* sizeof(int)]);

                    auto textureSize = (uint32_t)(m_Width*m_Height*m_NbChannels);
                    uint32_t compressedSize = (uint32_t)buffer.size() - sizeof(4*sizeof(int));

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

				void				setDataSourceAccount	(DataSourceAccount *dataSourceAccount)		{ m_dataSourceAccount = dataSourceAccount; }
				DataSourceAccount *	getDataSourceAccount	(void) const								{ return m_dataSourceAccount; }

            private:
                bool m_IsLoaded;
                bool m_IsLoading;
                int m_Width;
                int m_Height;
                int m_NbChannels;
                int m_Format;
                WString m_DataSource;
                condition_variable m_TextureCV;
                mutex m_TextureMutex;
                const scalable_mesh::azure::Storage* m_stream_store;

				DataSourceAccount *	m_dataSourceAccount;
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
                    texture.Load(getDataSourceAccount(), blockID.m_integerID);
                    }
                }
            assert(texture.IsLoaded());
            return texture;
            }

        StreamingTextureTileStore(DataSourceAccount *dataSourceAccount, WCharCP directory)
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

			setDataSourceAccount(dataSourceAccount);
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
            pi_uncompressedPacket.SetBuffer(DataTypeArray + 3 * sizeof(int), countData - 3 * sizeof(int)); // The data block starts with 12 bytes of metadata, followed by pixel data
            pi_uncompressedPacket.SetDataSize(countData - 3 * sizeof(int));
            // Retrieve width, height and number of channels from the first 12 bytes of the data block
            int w = ((int*)DataTypeArray)[0];
            int h = ((int*)DataTypeArray)[1];
            int nOfChannels = ((int*)DataTypeArray)[2];
            int format = 0; // Keep an int to define the format and possible other options
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, w, h, nOfChannels);
            bvector<uint8_t> texData(4 * sizeof(int) + pi_compressedPacket.GetDataSize());
            int *pHeader = (int*)(texData.data());
            pHeader[0] = w;
            pHeader[1] = h;
            pHeader[2] = nOfChannels;
            pHeader[3] = format;
            memcpy(texData.data() + 4 * sizeof(int), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
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
            assert(!texture.empty());
            ((int*)DataTypeArray)[0] = (int)texture.GetWidth();
            ((int*)DataTypeArray)[1] = (int)texture.GetHeight();
            ((int*)DataTypeArray)[2] = (int)texture.GetNbChannels();
            memcpy(DataTypeArray + 3 * sizeof(int), texture.data(), std::min(texture.size(), maxCountData));
            //return texture.size() + 3 * sizeof(int);
            return std::min(texture.size() + 3 * sizeof(int), maxCountData);
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return false;
            }

		void				setDataSourceAccount	(DataSourceAccount *dataSourceAccount)	{m_dataSourceAccount = dataSourceAccount;}
		DataSourceAccount *	getDataSourceAccount	(void) const							{return m_dataSourceAccount;}

    private:
        WString m_path;
        mutable map<uint32_t, Texture> m_textureCache;
        // NEEDS_WORK_SM_STREAMING: should only have one stream store for all data types
        scalable_mesh::azure::Storage m_stream_store;

		DataSourceAccount *	m_dataSourceAccount;
    };
