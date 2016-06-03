//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMStreamingTileStore.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



#include "SMPointTileStore.h"
#include <ImagePP/all/h/HCDCodecZlib.h>
#include "ScalableMesh/Streaming/AzureStorage.h"
#include <curl/curl.h>
#include <condition_variable>
#include <CloudDataSource/DataSourceAccount.h>
#ifdef VANCOUVER_API
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#endif

#ifndef NDEBUG
#define DEBUG_GROUPS
#endif


//static bool s_useStreamingStore = true;
extern bool s_stream_from_disk;
extern bool s_stream_from_file_server;
extern bool s_stream_from_grouped_store;
extern bool s_is_virtual_grouping;
extern uint32_t s_max_number_nodes_in_group;
extern size_t s_max_group_size;
extern size_t s_max_group_depth;
extern size_t s_max_group_common_ancestor;

extern std::mutex fileMutex;

struct PointBlock : public bvector<uint8_t> {
public:
    bool IsLoading() { return m_pIsLoading; }
    bool IsLoaded() { return m_pIsLoaded; }
    void LockAndWait()
        {
        unique_lock<mutex> lock(m_pPointBlockMutex);
        m_pPointBlockCV.wait(lock, [this]() { return m_pIsLoaded; });
        }
		
    void SetLoading() { m_pIsLoading = true; }

	DataSource *initializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer> &dest, DataSourceBuffer::BufferSize destSize)
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

	
    void Load(DataSourceAccount *dataSourceAccount)
    {
		if (!IsLoaded())
        {
			if (IsLoading())
            {
            	LockAndWait();
            }
            else
            {
				std::unique_ptr<DataSource::Buffer>			dest;
				DataSource								*	dataSource;
				DataSource::DataSize						readSize;

				DataSourceURL	dataSourceURL(m_DataSource.data());

				DataSourceBuffer::BufferSize	destSize = 5 * 1024 * 1024;

				dataSource = initializeDataSource(dataSourceAccount, dest, destSize);
				if (dataSource == nullptr)
					return;

				dataSource->open(dataSourceURL, DataSourceMode_Read);

				dataSource->read(dest.get(), destSize, readSize, 0);

				dataSource->close();

                m_pIsLoaded = true;
            }
        }
		
        assert(IsLoaded());
	}
	
    void SetLoaded()
        {
        m_pIsLoaded = true;
        m_pIsLoading = false;
        m_pPointBlockCV.notify_all();
        }
    void SetDataSource(const WString& pi_DataSource)
        {
        m_DataSource = pi_DataSource;
        }
    void SetStore(const scalable_mesh::azure::Storage& pi_Store)
        {
        m_stream_store = &pi_Store;
        }
    void DecompressPoints(uint8_t* pi_CompressedData, uint32_t pi_CompressedDataSize, uint32_t pi_UncompressedDataSize)
        {
        HPRECONDITION(pi_CompressedDataSize <= (numeric_limits<uint32_t>::max) ());

        this->resize(pi_UncompressedDataSize);

        HCDPacket uncompressedPacket(this->data(), pi_UncompressedDataSize, pi_UncompressedDataSize),
            compressedPacket(&pi_CompressedData[0], pi_CompressedDataSize, pi_CompressedDataSize);

        // initialize codec
        HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_CompressedDataSize);
        const size_t unCompressedDataSize = pCodec->DecompressSubset(compressedPacket.GetBufferAddress(),
                                                                   compressedPacket.GetDataSize(),
                                                                   uncompressedPacket.GetBufferAddress(),
                                                                   uncompressedPacket.GetBufferSize());
        assert(unCompressedDataSize != 0 && pi_UncompressedDataSize == uncompressedPacket.GetDataSize());
        uncompressedPacket.SetDataSize(unCompressedDataSize);
        }

private:
    struct MemoryStruct {
        bvector<Byte>* memory;
        size_t         size;
        };
    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *)userp;

        assert(mem->memory->capacity() >= mem->memory->size() + realsize);

        //    mem->memory->assign((Byte*)contents, (Byte*)contents + realsize);
        mem->memory->insert(mem->memory->end(), (Byte*)contents, (Byte*)contents + realsize);

        return realsize;
        }
    StatusInt LoadFromLocal(const WString& m_pFilename)
        {
        uint32_t uncompressedSize = 0;
        BeFile file;
        if (BeFileStatus::Success == OPEN_FILE_SHARE(file, m_pFilename.c_str(), BeFileAccess::Read))
            {

            size_t fileSize = 0;
            file.GetSize(fileSize);
            
            // Read uncompressed size
            uint32_t bytesRead = 0;
            auto read_result = file.Read(&uncompressedSize, &bytesRead, sizeof(uint32_t));
            assert(BeFileStatus::Success == read_result);
            assert(bytesRead == sizeof(uint32_t));
            
            // Read compressed points
            auto compressedSize = fileSize - sizeof(uint32_t);
            bvector<uint8_t> ptData(compressedSize);
            read_result = file.Read(&ptData[0], &bytesRead, (uint32_t)compressedSize);
            assert(bytesRead == compressedSize);
            assert(BeFileStatus::Success == read_result);
            file.Close();
            
            this->DecompressPoints(&ptData[0], (uint32_t)compressedSize, uncompressedSize);
            }
        else
            {
            //assert(!"Problem opening block of points for reading");
            file.Close();
            return ERROR_FILE_NOT_FOUND;
            }
        return SUCCESS;
        }
    StatusInt LoadFromAzure(const WString& m_pFilename)
        {
        bool blobDownloaded = false;
        m_stream_store->DownloadBlob(m_pFilename.c_str(), [this, &blobDownloaded](scalable_mesh::azure::Storage::point_buffer_type& buffer)
            {
            assert(!buffer.empty());

            uint32_t uncompressedSize = reinterpret_cast<uint32_t&>(buffer[0]);
            uint32_t sizeData = (uint32_t)buffer.size() - sizeof(uint32_t);
            this->DecompressPoints(buffer.data() + sizeof(uint32_t), sizeData, uncompressedSize);

            blobDownloaded = true;
            });
        return blobDownloaded ? SUCCESS : ERROR;
        }
    void LoadFromFileSystem(const WString& m_pFilename)
        {
        CURL *curl_handle;
        bool retCode = true;
        struct MemoryStruct chunk;
        const int maxCountData = 100000;
        Utf8String blockUrl = "http://realitydatastreaming.azurewebsites.net/Mesh/c1sub_scalablemesh/";
        Utf8String name;
        BeStringUtilities::WCharToUtf8(name, m_pFilename.c_str());
        blockUrl += name;

        chunk.memory = this;
        chunk.memory->reserve(maxCountData);
        chunk.size = 0;
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();

        // NEEDS_WORK_SM_STREAMING: Remove this when streaming works reasonably well
        //std::lock_guard<std::mutex> lock(fileMutex);
        //BeFile file;
        //if (BeFileStatus::Success == OPEN_FILE(file, L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\FitView.node", BeFileAccess::ReadWrite) ||
        //    BeFileStatus::Success == file.Create(L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\FitView.node"))
        //    {
        //    file.SetPointer(0, BeFileSeekOrigin::End);
        //    auto node_location = L"http://realitydatastreaming.azurewebsites.net/Mesh/c1sub_scalablemesh/" + block_name + L"\n";
        //    Utf8String utf8_node_location;
        //    BeStringUtilities::WCharToUtf8(utf8_node_location, node_location.c_str());
        //    file.Write(NULL, utf8_node_location.c_str(), (uint32_t)utf8_node_location.length());
        //    }
        //else
        //    {
        //    assert(!"Problem creating nodes file");
        //    }
        //
        //file.Close();

        curl_easy_setopt(curl_handle, CURLOPT_URL, blockUrl.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        //    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        /* get it! */
        CURLcode res = curl_easy_perform(curl_handle);
        /* check for errors */
        if (CURLE_OK != res)
            {
            retCode = false;
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            assert(false);
            }

        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();

        assert(!chunk.memory->empty() && chunk.memory->size() <= 1000000);

        uint32_t uncompressedSize = reinterpret_cast<uint32_t&>((*chunk.memory)[0]);
        uint32_t sizeData = (uint32_t)chunk.memory->size() - sizeof(uint32_t);

        this->DecompressPoints(&(*chunk.memory)[0] + sizeof(uint32_t), sizeData, uncompressedSize);
        }

private:
    bool m_pIsLoading = false;
    bool m_pIsLoaded = false;
    WString m_DataSource;
    const scalable_mesh::azure::Storage* m_stream_store;
    condition_variable m_pPointBlockCV;
    mutex m_pPointBlockMutex;
    };

struct SMNodeHeader {
    uint64_t blockid;
    uint32_t offset;
    uint64_t size;
    };

struct SMGroupHeader : public vector<SMNodeHeader>, public HFCShareableObject<SMGroupHeader> {
public:
    SMGroupHeader() : m_pGroupID(-1) {}
    SMGroupHeader(const size_t& pi_pGroupID) : m_pGroupID(pi_pGroupID) {}
    SMGroupHeader(const size_t& pi_pGroupID, const size_t& pi_pSize) : vector<SMNodeHeader>(pi_pSize), m_pGroupID(pi_pGroupID) {}

    size_t GetID() { return m_pGroupID; }
    void   SetID(const size_t& pi_pGroupID) { m_pGroupID = pi_pGroupID; }

    void AddNode(const SMNodeHeader& pi_pNodeHeader) { this->push_back(pi_pNodeHeader); }

private:
    size_t m_pGroupID;
    };

struct SMGroupNodeIds : vector<uint64_t> {
    uint64_t m_pSizeOfRawHeaders;
    };
class SMNodeGroupMasterHeader : public std::map<size_t, SMGroupNodeIds>, public HFCShareableObject<SMNodeGroupMasterHeader>
    {
    public:
        SMNodeGroupMasterHeader() {}

        void AddGroup(const size_t& pi_pGroupID, size_type pi_pCount = 10000)
            {
            auto& newGroup = this->operator[](pi_pGroupID);
            newGroup.reserve(s_max_number_nodes_in_group);
            }

        void AddNodeToGroup(const size_t& pi_pGroupID, const uint64_t& pi_pNodeID, const uint64_t& pi_pNodeHeaderSize)
            {
            auto& group = this->operator[](pi_pGroupID);
            group.push_back(pi_pNodeID);
            group.m_pSizeOfRawHeaders += pi_pNodeHeaderSize;
            }

        void SaveToFile(const WString pi_pOutputDirPath)
            {
            assert(!m_pOldMasterHeader.empty()); // Old master header must be set!

            wstringstream ss;
            ss << WString(pi_pOutputDirPath + L"/MasterHeaderWithGroups.bin");
            auto group_header_filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_header_filename.c_str(), BeFileAccess::Write) ||
                BeFileStatus::Success == file.Create(group_header_filename.c_str()))
                {
                uint32_t NbChars = 0;

                // Save old Master Header part: size + data
                const uint32_t sizeOldMasterHeaderFile = (uint32_t)m_pOldMasterHeader.size();
                file.Write(&NbChars, &sizeOldMasterHeaderFile, sizeof(sizeOldMasterHeaderFile));
                assert(NbChars == sizeof(sizeOldMasterHeaderFile));

                file.Write(&NbChars, m_pOldMasterHeader.data(), sizeOldMasterHeaderFile);
                assert(NbChars == (uint32_t)m_pOldMasterHeader.size());

                file.Write(&NbChars, &s_is_virtual_grouping, sizeof(s_is_virtual_grouping));

                // Append group information
                for (auto& group : *this)
                    {
                    // Group id
                    auto const id = group.first;
                    file.Write(&NbChars, &id, sizeof(id));
                    assert(NbChars == sizeof(id));

                    auto& groupInfo = group.second;

                    // Group total size of headers
                    if (s_is_virtual_grouping)
                        {
                        auto const total_size = groupInfo.m_pSizeOfRawHeaders;
                        file.Write(&NbChars, &total_size, sizeof(total_size));
                        assert(NbChars == sizeof(total_size));
                        }

                    // Group number of nodes
                    auto const numNodes = groupInfo.size();
                    file.Write(&NbChars, &numNodes, sizeof(numNodes));
                    assert(NbChars == sizeof(numNodes));

                    // Group node ids
                    file.Write(&NbChars, groupInfo.data(), (uint32_t)numNodes * sizeof(uint64_t));
                    assert(NbChars == (uint32_t)numNodes * sizeof(uint64_t));
                    }
                }
            file.Close();
            }

        void SetOldMasterHeaderData(SQLiteIndexHeader pi_pOldMasterHeader)
            {
            // Serialize master header
            m_pOldMasterHeader.resize(sizeof(pi_pOldMasterHeader));
            memcpy(m_pOldMasterHeader.data(), &pi_pOldMasterHeader, sizeof(pi_pOldMasterHeader));
            }
    private:
        bvector<uint8_t> m_pOldMasterHeader;
    };

class SMNodeGroup : public HFCShareableObject<SMNodeGroup>
    {
    template <typename Type, typename Queue = std::queue<Type>>
    class distributor : Queue, std::mutex, std::condition_variable {
        typename Queue::size_type capacity;
        bool done = false;
        std::vector<std::thread> threads;

    public:
        template<typename Function>
        distributor(Function function
                    , unsigned int concurrency = std::thread::hardware_concurrency()
                    //, unsigned int concurrency = 2
                    , typename Queue::size_type max_items_per_thread = 1
                    )
            : capacity{ concurrency * max_items_per_thread }
            {
            if (!concurrency)
                throw std::invalid_argument("Concurrency must be non-zero");
            if (!max_items_per_thread)
                throw std::invalid_argument("Max items per thread must be non-zero");

            for (unsigned int count{ 0 }; count < concurrency; count += 1)
                threads.emplace_back(static_cast<void (distributor::*)(Function)>
                                     (&distributor::consume), this, function);
            }

        distributor(distributor &&) = default;
        distributor &operator=(distributor &&) = delete;

        ~distributor()
            {
            Wait();
            }

        void operator()(Type &&value)
            {
            std::unique_lock<std::mutex> lock(*this);
            while (Queue::size() == capacity)
                {
#ifdef DEBUG_GROUPS
                //std::cout << "distributor queue is full, waiting for jobs to complete... " << this << std::endl;
#endif
                wait(lock);
                }
            Queue::push(std::forward<Type>(value));
            notify_one();
            }

        void Wait()
            {
                    {
                    std::lock_guard<std::mutex> guard(*this);
                    done = true;
                    notify_all();
                    }
                    for (auto &&thread : threads) if (thread.joinable()) thread.join();
            }

    private:
        template <typename Function>
        void consume(Function process)
            {
#ifdef DEBUG_GROUPS
            //std::cout << this << " is starting to process queue..." << Queue::size() << std::endl;
#endif
            std::unique_lock<std::mutex> lock(*this);
            while (true) {
                if (!Queue::empty()) {
                    Type item{ std::move(Queue::front()) };
                    Queue::pop();
                    notify_one();
                    lock.unlock();
                    process(item);
                    lock.lock();
                    }
                else if (done) {
#ifdef DEBUG_GROUPS
                    //std::cout << this << " done adding to queue, no more items can be added... " << this << std::endl;
#endif
                    break;
                    }
                else {
#ifdef DEBUG_GROUPS
                    //std::cout << "distributor queue is empty but not done yet... " << this << std::endl;
#endif
                    wait(lock);
                    }
                }
            }
        };
    public:

        SMNodeGroup(DataSourceAccount *dataSourceAccount, const size_t& pi_pID, const size_t& pi_pSize, const uint64_t& pi_pTotalSizeOfHeaders)
            : m_pGroupHeader(new SMGroupHeader(pi_pID, pi_pSize)),
            m_pRawHeaders(pi_pTotalSizeOfHeaders),
            m_stream_store(nullptr), m_dataSourceAccount(dataSourceAccount)
            {};

        SMNodeGroup(DataSourceAccount *dataSourceAccount, WString& pi_pDataSourceName, scalable_mesh::azure::Storage& pi_pStreamStore)
            {
            // reserve space for total number of nodes for this group
            m_pGroupHeader->reserve(s_max_number_nodes_in_group);
            m_pRawHeaders.reserve(3000 * s_max_number_nodes_in_group);
            m_pTotalSize = 2 * sizeof(size_t);
            m_pDataSourceName = pi_pDataSourceName + L"g_";
            m_stream_store = &pi_pStreamStore;

			setDataSourceAccount(dataSourceAccount);

            }

        SMNodeGroup(DataSourceAccount *dataSourceAccount, const WString pi_pOutputDirPath, const size_t& pi_pGroupLevel, const size_t& pi_pGroupID)
            : m_pOutputDirPath(pi_pOutputDirPath), m_pLevel(pi_pGroupLevel), m_pGroupHeader(new SMGroupHeader(pi_pGroupID))
            {
            // reserve space for total number of nodes for this group
            m_pGroupHeader->reserve(s_max_number_nodes_in_group);
            m_pRawHeaders.reserve(3000 * s_max_number_nodes_in_group);

            m_pTotalSize = 2 * sizeof(size_t);

			setDataSourceAccount(dataSourceAccount);
            }

        size_t GetLevel() { return m_pLevel; }

        void SetLevel(const size_t& pi_NewID) { m_pLevel = pi_NewID; }

        size_t GetID() { return m_pGroupHeader->GetID(); }

        void SetID(const size_t& pi_NewID) { m_pGroupHeader->SetID(pi_NewID); }

        void SetAncestor(const size_t& pi_pLevel) { m_pAncestor = pi_pLevel; }

        void SetDataSource(const WString& pi_pDataSourceName, scalable_mesh::azure::Storage& pi_pStreamStore)
            {
            m_pDataSourceName = pi_pDataSourceName;
            m_stream_store = &pi_pStreamStore;
            }

        size_t GetNumberNodes() { return m_pGroupHeader->size(); }

        size_t GetSizeOfHeaders() { return m_pRawHeaders.size(); }

        bvector<Byte>::pointer GetRawHeaders(const uint32_t& offset) { return m_pRawHeaders.data() + offset; }

        size_t GetTotalSize() { return m_pTotalSize; }

        void SetHeader(HFCPtr<SMGroupHeader> pi_pGroupHeader) { m_pGroupHeader = pi_pGroupHeader; }

        HFCPtr<SMGroupHeader> GetHeader() { return m_pGroupHeader; }

        void IncreaseDepth() { ++m_pDepth; }

        void DecreaseDepth()
            {
            assert(m_pDepth > 0);
            --m_pDepth;
            }

        WString GetFilePath() { return m_pOutputDirPath; }

        void Open(const size_t& pi_pGroupID) { SetID(pi_pGroupID); }

        void Close()
            {
            Save();
            Clear();
            }

        void Clear()
            {
            m_pGroupHeader->clear();
            m_pRawHeaders.clear();
            m_pTotalSize = 2 * sizeof(size_t);
            m_pAncestor = -1;
            }

        void AddNode(uint32_t pi_NodeID, const std::unique_ptr<Byte>& pi_Data, uint32_t pi_Size)
            {
            const auto oldSize = m_pRawHeaders.size();
            m_pGroupHeader->AddNode(SMNodeHeader{ pi_NodeID, (uint32_t)oldSize, pi_Size });
            m_pRawHeaders.resize(oldSize + pi_Size);
            memmove(&m_pRawHeaders[oldSize], pi_Data.get(), pi_Size);
            m_pTotalSize += pi_Size + sizeof(SMNodeHeader);
            }

        bool IsEmpty()
            {
            return m_pGroupHeader->empty();
            }

        bool IsFull()
            {
            //return GetNumberNodes() >= s_max_number_nodes_in_group;
            return GetTotalSize() >= s_max_group_size;
            }

        bool IsMaxDepthAchieved()
            {
            return m_pDepth >= s_max_group_depth;
            }

        bool IsCommonAncestorTooFar(const size_t& pi_pLevelRequested)
            {
            return (m_pAncestor == -1 ? false : pi_pLevelRequested >= s_max_group_common_ancestor + m_pAncestor);
            }

        bool IsLoaded() { return m_pIsLoaded; }


		void setDataSourceAccount(DataSourceAccount *dataSourceAccount)
		{
			m_dataSourceAccount = dataSourceAccount;
		}

		DataSourceAccount *getDataSourceAccount(void)
		{
			return m_dataSourceAccount;
		}


        void Save()
            {
            WString path(m_pOutputDirPath + L"\\g_");
            wstringstream ss;
            ss << path << this->GetID() << L".bin";
            auto group_filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_filename.c_str(), BeFileAccess::Write) ||
                BeFileStatus::Success == file.Create(group_filename.c_str()))
                {
                uint32_t NbChars = 0;
                auto id = this->GetID();
                file.Write(&NbChars, &id, sizeof(id));
                assert(NbChars == sizeof(id));

                if (s_is_virtual_grouping)
                    {
                    auto sizeHeaders = GetSizeOfHeaders();
                    file.Write(&NbChars, &sizeHeaders, sizeof(sizeHeaders));
                    assert(NbChars == sizeof(sizeHeaders));
                    }

                const auto numNodes = m_pGroupHeader->size();
                file.Write(&NbChars, &numNodes, sizeof(numNodes));
                assert(NbChars == sizeof(numNodes));

                file.Write(&NbChars, m_pGroupHeader->data(), (uint32_t)numNodes * sizeof(SMNodeHeader));
                assert(NbChars == numNodes * sizeof(SMNodeHeader));

                if (!s_is_virtual_grouping)
                    {
                    auto sizeHeaders = (uint32_t)GetSizeOfHeaders();
                    file.Write(&NbChars, GetRawHeaders(0), sizeHeaders * sizeof(uint8_t));
                    assert(NbChars == sizeHeaders * sizeof(uint8_t));
                    }
                }
            else
                {
                assert(!"Problem creating new group file");
                }

            file.Close();
            }


		DataSource *initializeDataSource(std::unique_ptr<DataSource::Buffer> &dest, DataSourceBuffer::BufferSize destSize)
		{
			if (getDataSourceAccount() == nullptr)
				return nullptr;
															// Get the thread's DataSource or create a new one
			DataSource *dataSource = getDataSourceAccount()->getOrCreateThreadDataSource();
			if (dataSource == nullptr)
				return nullptr;
															// Make sure caching is enabled for this DataSource
			dataSource->setCachingEnabled(true);

			dest.reset(new unsigned char[destSize]);
															// Return the DataSource
			return dataSource;
		}


		void Load()
		{
			unique_lock<mutex> lk(m_pGroupMutex);
			if (m_pIsLoading)
			{
				m_pGroupCV.wait(lk, [this] {return !m_pIsLoading; });
			}
			else
			{
				m_pIsLoading = true;
				
                if(s_is_virtual_grouping)
                {
					this->LoadGroupParallel();
				}
								
				std::unique_ptr<DataSource::Buffer>			dest;
				DataSource								*	dataSource;
				DataSource::DataSize						readSize;

				DataSourceBuffer::BufferSize				destSize = 5 * 1024 * 1024;

				dataSource = initializeDataSource(dest, destSize);
				if (dataSource == nullptr)
					return;
			
				loadFromDataSource(dataSource, dest.get(), destSize, readSize);				

				if(true)
				{
					uint32_t position = 0;
					size_t id;
					memcpy(&id, dest.get(), sizeof(size_t));
					assert(m_pGroupHeader->GetID() == id);
					position += sizeof(size_t);

					size_t numNodes;
					memcpy(&numNodes, dest.get() + position, sizeof(numNodes));
					assert(m_pGroupHeader->size() == numNodes);
					position += sizeof(numNodes);

					memcpy(m_pGroupHeader->data(), dest.get() + position, numNodes * sizeof(SMNodeHeader));
					position += (uint32_t)numNodes * sizeof(SMNodeHeader);

					const auto headerSectionSize = readSize - position;
					m_pRawHeaders.resize(headerSectionSize);
					memcpy(m_pRawHeaders.data(), dest.get() + position, headerSectionSize);
				}

				m_pIsLoading = false;
				m_pGroupCV.notify_all();
			}
			m_pIsLoaded = true;
		}


        StatusInt Load_Old()
            {
            unique_lock<mutex> lk(m_pGroupMutex);
            if (m_pIsLoading)
                {
                m_pGroupCV.wait(lk, [this] {return !m_pIsLoading; });
                }
            else {
                m_pIsLoading = true;
                if (s_is_virtual_grouping)
                    {
                    this->LoadGroupParallel();
                    }
                else {
                    std::unique_ptr<uint8_t> inBuffer = nullptr;
                    uint32_t bytes_read = 0;
                    m_pIsLoading = true;
                    if (s_stream_from_disk && SUCCESS != this->LoadFromLocal(inBuffer, bytes_read))
                        {
                        m_pIsLoading = false;
                        m_pGroupCV.notify_all();
                        return ERROR;
                        }
                    if (!s_stream_from_disk && SUCCESS != this->LoadFromAzure(inBuffer, bytes_read))
                        {
                        m_pIsLoading = false;
                        m_pGroupCV.notify_all();
                        return ERROR;
                        }
                    uint32_t position = 0;
                    size_t id;
                    memcpy(&id, inBuffer.get(), sizeof(size_t));
                    assert(m_pGroupHeader->GetID() == id);
                    position += sizeof(size_t);

                    size_t numNodes;
                    memcpy(&numNodes, inBuffer.get() + position, sizeof(numNodes));
                    assert(m_pGroupHeader->size() == numNodes);
                    position += sizeof(numNodes);

                    memcpy(m_pGroupHeader->data(), inBuffer.get() + position, numNodes * sizeof(SMNodeHeader));
                    position += (uint32_t)numNodes * sizeof(SMNodeHeader);

                    const auto headerSectionSize = bytes_read - position;
                    m_pRawHeaders.resize(headerSectionSize);
                    memcpy(m_pRawHeaders.data(), inBuffer.get() + position, headerSectionSize);

                    m_pIsLoading = false;
                    m_pGroupCV.notify_all();
                    }
                m_pIsLoading = false;
                m_pGroupCV.notify_all();
                }
            m_pIsLoaded = true;
            return SUCCESS;
            }

        void LoadGroupParallel()
            {
            uint64_t currentPosition = 0;
            int numProcessedNodeId = 0;
            std::mutex rawHeadersUpdateMutex;
            distributor<uint64_t> nodeLoader([this, &currentPosition, &numProcessedNodeId, &rawHeadersUpdateMutex](uint64_t nodeId)
                {
                ++numProcessedNodeId;
//#ifdef DEBUG_GROUPS
//                std::cout << "Processing... " << nodeId << std::endl;
//#endif
                uint8_t* rawHeader = new uint8_t[10000];
                auto& nodeHeader = this->GetNodeHeader(nodeId);
                auto headerSize = this->GetSingleNodeFromStore(nodeId, rawHeader);
                std::unique_lock<std::mutex> lock(rawHeadersUpdateMutex);
                nodeHeader.size = headerSize;
                nodeHeader.offset = currentPosition;
                memmove(this->m_pRawHeaders.data() + currentPosition, rawHeader, nodeHeader.size);
                currentPosition += nodeHeader.size;
                delete rawHeader;
                });
            for (auto nodeHeader : *m_pGroupHeader) nodeLoader(std::move(nodeHeader.blockid));
#ifdef DEBUG_GROUPS
            std::cout << "waiting for nodes to process..." << std::endl;
#endif
            nodeLoader.Wait();
#ifdef DEBUG_GROUPS
            std::cout << "num processed node Ids: " << numProcessedNodeId << std::endl;
#endif
            }

        bool ContainsNode(const uint64_t& pi_pNodeID)
            {
            assert(!m_pGroupHeader->empty());
            auto node = std::find_if(begin(*m_pGroupHeader), end(*m_pGroupHeader), [&](SMNodeHeader& nodeId)
                {
                return nodeId.blockid == pi_pNodeID;
                });
            return node != m_pGroupHeader->end();
            }

        SMNodeHeader& GetNodeHeader(const uint64_t& pi_pNodeHeaderID)
            {
            return *(std::find_if(begin(*m_pGroupHeader), end(*m_pGroupHeader), [&](SMNodeHeader& nodeId)
                {
                return pi_pNodeHeaderID == nodeId.blockid;
                }));
            }

    private:

		void loadFromDataSource(DataSource *dataSource, DataSource::Buffer *dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize)
		{
			if (dataSource == nullptr)
				return;

			wstringstream			ss;

			ss << m_pDataSourceName << this->GetID() << L".bin";
			auto groupFilename = ss.str();

			DataSourceURL	dataSourceURL(groupFilename);

			dataSource->open(dataSourceURL, DataSourceMode_Read);

			dataSource->read(dest, destSize, readSize, 0);

			dataSource->close();
		}
		
        uint64_t GetSingleNodeFromStore(const uint64_t& pi_pNodeID, uint8_t* pi_pData)
		{

			std::unique_ptr<DataSource::Buffer>			dest;
			DataSource								*	dataSource;
			DataSource::DataSize						readSize;
			DataSourceBuffer::BufferSize				destSize = 5 * 1024 * 1024;
            wstringstream 								ss;
			
            ss << m_pDataSourceName << L"n_" << pi_pNodeID << L".bin";
			DataSourceURL	dataSourceURL(ss.str());

			dataSource = initializeDataSource(dest, destSize);
			if (dataSource == nullptr)
				return 0;

			dataSource->open(dataSourceURL, DataSourceMode_Read);

			dataSource->read(dest.get(), destSize, readSize, 0);

			dataSource->close();

			if(readSize > 0)
			{						
				memmove(pi_pData, reinterpret_cast<char *>(dest.get()), readSize);
			}
			
			return readSize;			
    	}
		

        uint64_t GetSingleNodeFromStore_Old(const uint64_t& pi_pNodeID, uint8_t* pi_pData)
            {
            //static set<uint64_t> nodeIds;
            //assert(nodeIds.insert(pi_pNodeID).second);
            //static int nbDownloadedNodeHeaders = 0;
            //++nbDownloadedNodeHeaders;
            //std::cout << "total node headers fetched: " << nbDownloadedNodeHeaders << std::endl;
            wstringstream ss;
            ss << m_pDataSourceName << L"n_" << pi_pNodeID << L".bin";
            auto filename = ss.str();
            if (s_stream_from_disk)
                {
                BeFile file;
                if (BeFileStatus::Success != OPEN_FILE(file, filename.c_str(), BeFileAccess::Read))
                    {
                    assert(false); // node header file must exist
                    }
                uint64_t fileSize;
                file.GetSize(fileSize);
                bvector<uint8_t> inBuffer(fileSize);
                uint32_t bytes_read = 0;
                file.Read(pi_pData, &bytes_read, fileSize);
                assert(bytes_read == fileSize);
                return fileSize;
                //PointBlock block;
                //block.SetDataSource(filename.c_str());
                //block.SetStore(*m_stream_store);
                //block.Load();
                }
            else
                {
                uint64_t dataSize = 0;
                m_stream_store->DownloadBlob(filename.c_str(), [&pi_pData, &dataSize](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                    {
                    if (buffer.empty())
                        {
                        dataSize = 0;
                        return;
                        }
                    dataSize = (uint64_t)buffer.size();

                    //pi_pData = new uint8_t[dataSize];
                    memmove(pi_pData, buffer.data(), dataSize);
                    });
                return dataSize;
                }
            }
        StatusInt LoadFromLocal(std::unique_ptr<uint8_t>& pi_pBuffer, uint32_t& pi_pBufferSize)
            {
            wstringstream ss;
            ss << m_pDataSourceName << this->GetID() << L".bin";
            auto group_filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_filename.c_str(), BeFileAccess::Read))
                {
                uint64_t fileSize;
                file.GetSize(fileSize);
                pi_pBuffer.reset(new uint8_t[fileSize]);
                file.Read(pi_pBuffer.get(), &pi_pBufferSize, (uint32_t)fileSize);
                assert(pi_pBufferSize == fileSize);
                }
            else
                {
                return ERROR_FILE_NOT_FOUND;
                }

            file.Close();
            return SUCCESS;
            }

        StatusInt LoadFromAzure(std::unique_ptr<uint8_t>& pi_pBuffer, uint32_t& pi_pBufferSize)
            {
            assert(m_stream_store != nullptr);

            wstringstream ss;
            ss << m_pDataSourceName << this->GetID() << L".bin";
            auto group_filename = ss.str();
            StatusInt status;
            m_stream_store->DownloadBlob(group_filename.c_str(), [&pi_pBuffer, &pi_pBufferSize, &status](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                {
                if (buffer.empty())
                    {
                    pi_pBufferSize = 0;
                    status = ERROR_EMPTY;
                    return;
                    }
                status = SUCCESS;
                pi_pBufferSize = (uint32_t)buffer.size();

                pi_pBuffer.reset(new uint8_t[pi_pBufferSize]);
                memcpy(pi_pBuffer.get(), buffer.data(), pi_pBufferSize);
                });
            return status;
            }

    private:
        bool   m_pIsLoaded = false;
        bool   m_pIsLoading = false;
        size_t m_pLevel = 0;
        size_t m_pTotalSize;
        size_t m_pNumLevels = 0;
        size_t m_pDepth = 0;
        size_t m_pAncestor = -1;
        bvector<uint8_t> m_pRawHeaders;
        WString m_pOutputDirPath;
        WString m_pDataSourceName;
        HFCPtr<SMGroupHeader> m_pGroupHeader;
        scalable_mesh::azure::Storage* m_stream_store;
        condition_variable m_pGroupCV;
        mutex m_pGroupMutex;

		DataSourceAccount *m_dataSourceAccount;
    };



template <typename POINT, typename EXTENT> class SMStreamingPointTaggedTileStore : public SMPointTileStore<POINT, EXTENT>// , public HFCShareableObject<SMPointTileStore<POINT, EXTENT> >
    {

    private:
        static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
            }

        static IDTMFile::NodeID ConvertChildID(const HPMBlockID& childID)
            {
            return static_cast<IDTMFile::NodeID>(childID.m_integerID);
            }

        static IDTMFile::NodeID ConvertNeighborID(const HPMBlockID& neighborID)
            {
            return static_cast<IDTMFile::NodeID>(neighborID.m_integerID);
            }

        static bool IsValidID(const HPMBlockID& blockID)
            {
            return blockID.IsValid() && blockID.m_integerID != IDTMFile::GetNullNodeID();
            }

        bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
                                   HCDPacket& pi_compressedPacket) const
            {
            HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

            // initialize codec
            HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_uncompressedPacket.GetDataSize());
            pi_compressedPacket.SetBufferOwnership(true);
            size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
            pi_compressedPacket.SetBuffer(new Byte[compressedBufferSize], compressedBufferSize * sizeof(Byte));
            const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(Byte), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(Byte));
            pi_compressedPacket.SetDataSize(compressedDataSize);

            return true;
            }

        HFCPtr<SMNodeGroup> FindGroup(HPMBlockID blockID)
            {
            auto nodeIDToFind = this->ConvertBlockID(blockID);
            for (auto& group : m_nodeHeaderGroups)
                {
                if (group->ContainsNode(nodeIDToFind))
                    {
                    return group;
                    }
                }

            return nullptr;
            }

        HFCPtr<SMNodeGroup> GetGroup(HPMBlockID blockID)
            {
            auto group = this->FindGroup(blockID);
            assert(group != nullptr);
            if (!group->IsLoaded())
                {
                group->Load();
                }
            return group;
            }

        void ReadNodeHeaderFromBinary(SMPointNodeHeader<EXTENT>* header, uint8_t* headerData, uint64_t& maxCountData) const
            {
            size_t dataIndex = 0;

            memcpy(&header->m_filtered, headerData + dataIndex, sizeof(header->m_filtered));
            dataIndex += sizeof(header->m_filtered);
            uint32_t parentNodeID;
            memcpy(&parentNodeID, headerData + dataIndex, sizeof(parentNodeID));
            header->m_parentNodeID = parentNodeID != IDTMFile::GetNullNodeID() ? HPMBlockID(parentNodeID) : IDTMFile::GetNullNodeID();
            dataIndex += sizeof(parentNodeID);
            uint32_t subNodeNoSplitID;
            memcpy(&subNodeNoSplitID, headerData + dataIndex, sizeof(subNodeNoSplitID));
            header->m_SubNodeNoSplitID = subNodeNoSplitID != IDTMFile::GetNullNodeID() ? HPMBlockID(subNodeNoSplitID) : IDTMFile::GetNullNodeID();
            dataIndex += sizeof(subNodeNoSplitID);
            memcpy(&header->m_level, headerData + dataIndex, sizeof(header->m_level));
            dataIndex += sizeof(header->m_level);
            memcpy(&header->m_IsBranched, headerData + dataIndex, sizeof(header->m_IsBranched));
            dataIndex += sizeof(header->m_IsBranched);
            memcpy(&header->m_IsLeaf, headerData + dataIndex, sizeof(header->m_IsLeaf));
            dataIndex += sizeof(header->m_IsLeaf);
            memcpy(&header->m_SplitTreshold, headerData + dataIndex, sizeof(header->m_SplitTreshold));
            dataIndex += sizeof(header->m_SplitTreshold);
            memcpy(&header->m_totalCount, headerData + dataIndex, sizeof(header->m_totalCount));
            dataIndex += sizeof(header->m_totalCount);
            memcpy(&header->m_nodeCount, headerData + dataIndex, sizeof(header->m_nodeCount));
            dataIndex += sizeof(header->m_nodeCount);
            memcpy(&header->m_arePoints3d, headerData + dataIndex, sizeof(header->m_arePoints3d));
            dataIndex += sizeof(header->m_arePoints3d);
            memcpy(&header->m_isTextured, headerData + dataIndex, sizeof(header->m_isTextured));
            dataIndex += sizeof(header->m_isTextured);
            memcpy(&header->m_nbFaceIndexes, headerData + dataIndex, sizeof(header->m_nbFaceIndexes));
            dataIndex += sizeof(header->m_nbFaceIndexes);
            uint32_t graphID;
            memcpy(&graphID, headerData + dataIndex, sizeof(graphID));
            header->m_graphID = graphID != IDTMFile::GetNullNodeID() ? HPMBlockID(graphID) : IDTMFile::GetNullNodeID();
            dataIndex += sizeof(graphID);

            memcpy(&header->m_nodeExtent, headerData + dataIndex, 6*sizeof(double));
            dataIndex += 6 * sizeof(double);

            memcpy(&header->m_contentExtentDefined, headerData + dataIndex, sizeof(header->m_contentExtentDefined));
            dataIndex += sizeof(header->m_contentExtentDefined);
            if (header->m_contentExtentDefined)
                {
                memcpy(&header->m_contentExtent, headerData + dataIndex, 6 * sizeof(double));
                dataIndex += 6 * sizeof(double);
                }

            /* Indices */
            uint32_t idx;
            memcpy(&idx, headerData + dataIndex, sizeof(idx));
            dataIndex += sizeof(idx);
            header->m_ptsIndiceID.resize(1);
            header->m_ptsIndiceID[0] = idx;

            if (header->m_isTextured)
                {
                header->m_textureID.resize(1);
                header->m_textureID[0] = IDTMFile::GetNullNodeID();
                header->m_ptsIndiceID.resize(2);
                header->m_ptsIndiceID[1] = (int)idx;
                header->m_ptsIndiceID[0] = SQLiteNodeHeader::NO_NODEID;
                header->m_nbTextures = 1;
                header->m_uvsIndicesID.resize(1);
                header->m_uvsIndicesID[0] = idx;
                }

            /* Mesh components */
            size_t numberOfMeshComponents;
            memcpy(&numberOfMeshComponents, headerData + dataIndex, sizeof(numberOfMeshComponents));
            dataIndex += sizeof(numberOfMeshComponents);
            header->m_numberOfMeshComponents = numberOfMeshComponents;
            header->m_meshComponents = new int[numberOfMeshComponents];
            for (size_t componentIdx = 0; componentIdx < header->m_numberOfMeshComponents; componentIdx++)
                {
                int component;
                memcpy(&component, headerData + dataIndex, sizeof(component));
                dataIndex += sizeof(component);
                header->m_meshComponents[componentIdx] = component;
                }

            /* Clips */
            uint32_t nbClipSetsIDs;
            memcpy(&nbClipSetsIDs, headerData + dataIndex, sizeof(nbClipSetsIDs));
            dataIndex += sizeof(nbClipSetsIDs);
            header->m_clipSetsID.clear();
            header->m_clipSetsID.reserve(nbClipSetsIDs);
            for (size_t i = 0; i < nbClipSetsIDs; ++i)
                {
                uint32_t clip;
                memcpy(&clip, headerData + dataIndex, sizeof(clip));
                dataIndex += sizeof(clip);
                header->m_clipSetsID.push_back(clip != IDTMFile::GetNullNodeID() ? HPMBlockID(clip) : IDTMFile::GetNullNodeID());
                }

            /* Children */
            size_t nbChildren;
            memcpy(&nbChildren, headerData + dataIndex, sizeof(nbChildren));
            dataIndex += sizeof(nbChildren);
            header->m_apSubNodeID.clear();
            header->m_apSubNodeID.reserve(nbChildren);
            header->m_numberOfSubNodesOnSplit = nbChildren;
            for (size_t childInd = 0; childInd < nbChildren; childInd++)
                {
                uint32_t childID;
                memcpy(&childID, headerData + dataIndex, sizeof(childID));
                dataIndex += sizeof(childID);
                header->m_apSubNodeID.push_back(childID != IDTMFile::GetNullNodeID() ? HPMBlockID(childID) : IDTMFile::GetNullNodeID());
                }

            /* Neighbors */
            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {
                size_t numNeighbors;
                memcpy(&numNeighbors, headerData + dataIndex, sizeof(numNeighbors));
                dataIndex += sizeof(numNeighbors);
                header->m_apNeighborNodeID[neighborPosInd].clear();
                header->m_apNeighborNodeID[neighborPosInd].reserve(numNeighbors);
                for (size_t neighborInd = 0; neighborInd < numNeighbors; neighborInd++)
                    {
                    uint32_t neighborId;
                    memcpy(&neighborId, headerData + dataIndex, sizeof(neighborId));
                    dataIndex += sizeof(neighborId);
                    header->m_apNeighborNodeID[neighborPosInd].push_back(neighborId != IDTMFile::GetNullNodeID() ? HPMBlockID(neighborId) : IDTMFile::GetNullNodeID());
                    }
                }
            assert(dataIndex == maxCountData);
            }

        void GetNodeHeaderBinary(const HPMBlockID& blockID, std::unique_ptr<uint8_t>& po_pBinaryData, uint64_t& po_pDataSize)
            {
            //NEEDS_WORK_SM_STREAMING : are we loading node headers multiple times?
            std::lock_guard<std::mutex> lock(headerLock);
            wstringstream ss;
            ss << m_pathToHeaders << L"n_" << ConvertBlockID(blockID) << L".bin";

            auto filename = ss.str();

            bvector<uint8_t> headerBuffer;
            if (s_stream_from_disk)
                {
                BeFile file;
                if (BeFileStatus::Success != OPEN_FILE(file, filename.c_str(), BeFileAccess::Read))
                    {
                    assert(false); // node header file must exist
                    return;
                    }

                file.GetSize(po_pDataSize);
                po_pBinaryData.reset(new uint8_t[po_pDataSize]);

                uint32_t bytes_read = 0;
                file.Read(po_pBinaryData.get(), &bytes_read, po_pDataSize);
                assert(po_pDataSize == bytes_read);
                }
            else if (s_stream_from_file_server)
                {
                // NEEDS_WORK_SM_STREAMING: deactivate streaming from file server for now.
                assert(!"Streaming from file server is deactivated for the moment...");
                //ss << L".bin";
                //auto blob_name = ss.str();
                //bvector<Byte> buffer;
                //DownloadBlockFromFileServer(blob_name.c_str(), &buffer, 100000);
                //assert(!buffer.empty() && buffer.size() <= 100000);
                //Json::Reader reader;
                //reader.parse(reinterpret_cast<char*>(&buffer.front()), reinterpret_cast<char*>(&buffer.back()), result);
                }
            else {
                auto blob_name = filename.c_str();
                m_stream_store.DownloadBlob(blob_name, [&po_pBinaryData, &po_pDataSize](scalable_mesh::azure::Storage::point_buffer_type& buffer)
                    {
                    assert(!buffer.empty());
                    po_pDataSize = buffer.size();
                    po_pBinaryData.reset(new uint8_t[po_pDataSize]);
                    memmove(po_pBinaryData.get(), buffer.data(), po_pDataSize);
                    //Json::Reader reader;
                    //reader.parse(reinterpret_cast<char*>(&buffer.front()), reinterpret_cast<char*>(&buffer.back()), result);
                    });
                }
            }

        void ReadNodeHeaderFromJSON(SMPointNodeHeader<EXTENT>* header, const Json::Value& nodeHeader) const
            {
            auto& nodeExtent = nodeHeader["nodeExtent"];
            assert(nodeExtent.isObject());
            ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, nodeExtent["xMin"].asDouble());
            ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, nodeExtent["yMin"].asDouble());
            ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, nodeExtent["zMin"].asDouble());
            ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, nodeExtent["xMax"].asDouble());
            ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, nodeExtent["yMax"].asDouble());
            ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, nodeExtent["zMax"].asDouble());

            header->m_contentExtentDefined = nodeHeader["contentExtent"].asBool();
            if (header->m_contentExtentDefined)
                {
                auto& contentExtent = nodeHeader["contentExtent"];
                assert(contentExtent.isObject());
                ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, contentExtent["xMin"].asDouble());
                ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, contentExtent["yMin"].asDouble());
                ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, contentExtent["zMin"].asDouble());
                ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, contentExtent["xMax"].asDouble());
                ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, contentExtent["yMax"].asDouble());
                ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, contentExtent["zMax"].asDouble());
                }

            header->m_level = nodeHeader["resolution"].asUInt();
            header->m_filtered = nodeHeader["filtered"].asBool();
            uint32_t parentNodeID = nodeHeader["parentID"].asUInt();
            header->m_parentNodeID = parentNodeID != IDTMFile::GetNullNodeID() ? HPMBlockID(parentNodeID) : IDTMFile::GetNullNodeID();
            header->m_numberOfSubNodesOnSplit = nodeHeader["nbChildren"].asUInt();
            header->m_apSubNodeID.resize(header->m_numberOfSubNodesOnSplit);
            //header->m_IsLeaf = nodeHeader["isLeaf"].asBool();
            auto& children = nodeHeader["children"];
            assert(children.isArray());
            if (/*!header->m_IsLeaf &&*/ children.size() > 0)
                {
                for (auto& child : children)
                    {
                    auto childInd = child["index"].asUInt();
                    auto nodeId = child["id"].asUInt();
                    if (nodeId != IDTMFile::GetNullNodeID()) header->m_apSubNodeID[childInd] = HPMBlockID(nodeId);
                    }
                }
            header->m_IsLeaf = header->m_apSubNodeID.size() == 0 || (!header->m_apSubNodeID[0].IsValid());
            header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID.size() > 1 && header->m_apSubNodeID[1].IsValid());
            if (!header->m_IsLeaf && !header->m_IsBranched) header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];

            auto& neighbors = nodeHeader["neighbors"];
            assert(neighbors.isArray());
            if (neighbors.size() > 0)
                {
                for (auto& neighbor : neighbors)
                    {
                    assert(neighbor.isObject());
                    auto nodePos = neighbor["nodePos"].asUInt();
                    auto nodeId = neighbor["nodeId"].asUInt();
                    header->m_apNeighborNodeID[nodePos].push_back(nodeId);
                    }
                }

            //header->m_IsBranched = nodeHeader["isBranched"].asBool();
            header->m_balanced = nodeHeader["isBalanced"].asBool();
            header->m_SplitTreshold = nodeHeader["splitThreshold"].asUInt();
            header->m_totalCountDefined = true;
            header->m_totalCount = nodeHeader["totalCount"].asUInt();
            header->m_nodeCount = nodeHeader["nodeCount"].asUInt();
            header->m_arePoints3d = nodeHeader["arePoints3d"].asBool();
            header->m_nbFaceIndexes = nodeHeader["nbFaceIndexes"].asUInt();
            header->m_graphID = nodeHeader["graphID"].asUInt();
            header->m_uvID = nodeHeader["uvID"].asUInt();
            header->m_isTextured = nodeHeader["areTextured"].asBool();

            if (header->m_isTextured)
                {
                header->m_textureID.resize(1);
                header->m_textureID[0] = HPMBlockID(header->m_uvID);

                auto& indices = nodeHeader["indiceID"];
                assert(indices.isArray());
                header->m_ptsIndiceID.resize(indices.size());
                if (indices.size() > 0)
                    {
                    for (size_t indiceID = 0; indiceID < (size_t)indices.size(); indiceID++)
                        {
                        auto id = indices[(Json::ArrayIndex)indiceID].asUInt();
                        header->m_ptsIndiceID[indiceID] = id != IDTMFile::GetNullNodeID() ? HPMBlockID(id) : IDTMFile::GetNullNodeID();
                        }
                    }

                auto& uvIndiceIDs = nodeHeader["uvIndiceIDs"];
                assert(uvIndiceIDs.isArray());
                header->m_uvsIndicesID.resize(uvIndiceIDs.size());
                if (uvIndiceIDs.size() > 0)
                    {
                    for (size_t indiceID = 0; indiceID < (size_t)uvIndiceIDs.size(); indiceID++)
                        {
                        auto uvIndiceID = uvIndiceIDs[(Json::ArrayIndex)indiceID].asUInt();
                        header->m_uvsIndicesID[indiceID] = uvIndiceID != IDTMFile::GetNullNodeID() ? HPMBlockID(uvIndiceID) : IDTMFile::GetNullNodeID();
                        }
                    }
                else
                    {
                    header->m_uvsIndicesID.resize(1);
                    header->m_uvsIndicesID[0] = HPMBlockID();
                    }
                }


            //if (ConvertBlockID(header->m_uvID) == IDTMFile::GetNullNodeID()) header->m_uvID = HPMBlockID();
            //if (ConvertBlockID(header->m_graphID) == IDTMFile::GetNullNodeID()) header->m_graphID = HPMBlockID();
            header->m_numberOfMeshComponents = (size_t)nodeHeader["numberOfMeshComponents"].asUInt();
            auto& meshComponents = nodeHeader["meshComponents"];
            assert(meshComponents.isArray());
            header->m_meshComponents = new int[header->m_numberOfMeshComponents];
            if (header->m_numberOfMeshComponents > 0)
                {
                for (size_t i = 0; i < (size_t)header->m_numberOfMeshComponents; i++)
                    {
                    header->m_meshComponents[i] = meshComponents[(Json::ArrayIndex)i].asInt();
                    }
                }

            auto& clipSets = nodeHeader["clipSetsID"];
            assert(clipSets.isArray());
            header->m_clipSetsID.resize(clipSets.size());
            if (header->m_clipSetsID.size() > 0)
                {
                for (size_t i = 0; i < header->m_clipSetsID.size(); ++i) header->m_clipSetsID[i] = clipSets[(Json::ArrayIndex)i].asInt();
                }

            }

		DataSource *initializeDataSource(std::unique_ptr<DataSource::Buffer> &dest, DataSourceBuffer::BufferSize destSize) const
		{
			if (getDataSourceAccount() == nullptr)
				return nullptr;
															// Get the thread's DataSource or create a new one
			DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource();
			if (dataSource == nullptr)
				return nullptr;
															// Make sure caching is enabled for this DataSource
			dataSource->setCachingEnabled(true);

			dest.reset(new unsigned char[destSize]);
															// Return the DataSource
			return dataSource;
		}
		
		
        Json::Value GetNodeHeaderJSON(HPMBlockID blockID)
            {
            uint64_t headerSize = 0;
            std::unique_ptr<Byte> headerData = nullptr;
            this->GetNodeHeaderBinary(blockID, headerData, headerSize);
            Json::Value result;
            Json::Reader().parse(reinterpret_cast<char*>(headerData.get()), reinterpret_cast<char*>(headerData.get() + headerSize), result);
            return result;
            }

        PointBlock& GetBlock(HPMBlockID blockID) const
            {
            PointBlock& block = m_countInfo[blockID.m_integerID];
            if (!block.IsLoaded())
                {
                wstringstream ss;
                ss << m_path << L"p_" << blockID.m_integerID << L".bin";
                auto filename = ss.str();
                block.SetDataSource(filename.c_str());
                block.SetStore(m_stream_store);
                block.Load(getDataSourceAccount());
                }
            assert(block.IsLoaded());
            return block;
            }

	protected:

		DataSourceAccount *m_dataSourceAccount;

    public:
        // Constructor / Destructor

        SMStreamingPointTaggedTileStore(DataSourceAccount *dataSourceAccount, const WString& path, bool compress = true, bool haveHeaders = false, WString headers_path = L"", bool areNodeHeadersGrouped = false)
            :m_path(path),
            m_pathToHeaders(headers_path),
            m_use_node_header_grouping(areNodeHeadersGrouped),
            m_storage_connection_string(L"DefaultEndpointsProtocol=https;AccountName=pcdsustest;BlobEndpoint=https://scalablemesh.azureedge.net;AccountKey=3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ=="),
            //m_storage_connection_string(L"DefaultEndpointsProtocol=https;AccountName=pcdsustest;AccountKey=3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ=="),
            m_stream_store(m_storage_connection_string.c_str(), L"scalablemeshtest")
            {

				setDataSourceAccount(dataSourceAccount);

	            if (s_stream_from_disk)
                {
                // Create base directory structure to store information if not already done
                // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only
					if (0 == CreateDirectoryW(m_path.c_str(), NULL))
					{
						assert(ERROR_PATH_NOT_FOUND != GetLastError());
					}

                if (haveHeaders)
                {
					if (m_pathToHeaders.empty())
                    {
                        // Set default path to headers relative to points path
						m_pathToHeaders = m_path + L"../headers/";
					}
                    if (0 == CreateDirectoryW(m_pathToHeaders.c_str(), NULL))
                    {
						assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }
				}
            }
            else
			{
                // stream from azure
            }
		}

        virtual ~SMStreamingPointTaggedTileStore()
            {
            }

		void setDataSourceAccount(DataSourceAccount *dataSourceAccount)
		{
			m_dataSourceAccount = dataSourceAccount;
		}


		DataSourceAccount *getDataSourceAccount(void) const
		{
			return m_dataSourceAccount;
		}

        virtual bool HasSpatialReferenceSystem()
            {
            // NEEDS_WORK_SM_STREAMING : Add check to spatial reference system
            assert(!"TODO!");
            return false;
            }


        // New function
        virtual std::string GetSpatialReferenceSystem()
            {
            // NEEDS_WORK_SM_STREAMING : Add check to spatial reference system
            assert(!"TODO!");
            return string("");
            }

        // ITileStore interface
        virtual void Close()
            {
            }

        virtual bool StoreMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
            {
            if (indexHeader != NULL && indexHeader->m_rootNodeBlockID.IsValid())
                {
                Json::Value masterHeader;
                masterHeader["balanced"] = indexHeader->m_balanced;
                masterHeader["depth"] = (uint32_t)indexHeader->m_depth;
                masterHeader["rootNodeBlockID"] = ConvertBlockID(indexHeader->m_rootNodeBlockID);
                masterHeader["splitThreshold"] = indexHeader->m_SplitTreshold;
                masterHeader["singleFile"] = false;
                masterHeader["isTerrain"] = true;

                // Write to file
                auto filename = (m_path + L"..\\MasterHeader.sscm").c_str();
                BeFile file;
                uint64_t buffer_size;
                auto jsonWriter = [&file, &indexHeader, &buffer_size](BeFile& file, Json::Value& object) {

                    Json::StyledWriter writer;
                    auto buffer = writer.write(object);
                    buffer_size = buffer.size();
                    file.Write(NULL, buffer.c_str(), buffer_size);
                    };
                if (BeFileStatus::Success == OPEN_FILE(file, filename, BeFileAccess::Write))//file.Open(filename, BeFileAccess::Write, BeFileSharing::None))
                    {
                    jsonWriter(file, masterHeader);
                    }
                else if (BeFileStatus::Success == file.Create(filename))
                    {
                    jsonWriter(file, masterHeader);
                    }
                else
                    {
                    assert(!"Problem creating master header file");
                    }
                file.Close();
                }

            return true;
            }

		virtual size_t LoadMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
		{
			std::unique_ptr<DataSource::Buffer>			dest;
			DataSource								*	dataSource;
			DataSource::DataSize						readSize;
			DataSourceBuffer::BufferSize				destSize = 5 * 1024 * 1024;

			if (indexHeader != NULL)
			{

				if (m_use_node_header_grouping || s_stream_from_grouped_store)
				{
					wstringstream ss;
					ss << m_path << L"../MasterHeaderWithGroups.bin";

					DataSourceURL	dataSourceURL(ss.str());

					if (m_nodeHeaderGroups.empty())
					{
						dataSource = initializeDataSource(dest, destSize);
						if (dataSource == nullptr)
							return 0;

						if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
							return 0;

						if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
							return 0;

						dataSource->close();

						headerSize = readSize;

						uint64_t position = 0;

						uint32_t sizeOfOldMasterHeaderPart;
						memcpy(&sizeOfOldMasterHeaderPart, dest.get() + position, sizeof(sizeOfOldMasterHeaderPart));
						position += sizeof(sizeOfOldMasterHeaderPart);
						assert(sizeOfOldMasterHeaderPart == sizeof(SQLiteIndexHeader));

						SQLiteIndexHeader oldMasterHeader;
						memcpy(&oldMasterHeader, dest.get() + position, sizeof(SQLiteIndexHeader));
						position += sizeof(SQLiteIndexHeader);
						indexHeader->m_SplitTreshold = oldMasterHeader.m_SplitTreshold;
						indexHeader->m_balanced = oldMasterHeader.m_balanced;
						indexHeader->m_depth = oldMasterHeader.m_depth;
                        indexHeader->m_isTerrain = oldMasterHeader.m_isTerrain;
						indexHeader->m_singleFile = oldMasterHeader.m_singleFile;
                        assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

						auto rootNodeBlockID = oldMasterHeader.m_rootNodeBlockID;
						indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

                        memcpy(&s_is_virtual_grouping, reinterpret_cast<char *>(dest.get()) + position, sizeof(s_is_virtual_grouping));
                        position += sizeof(s_is_virtual_grouping);


						// Parse rest of file -- group information
						while (position < headerSize)
						{
                            size_t group_id;
                            memcpy(&group_id, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_id));
                            position += sizeof(group_id);

                            uint64_t group_totalSizeOfHeaders(0);
                            if (s_is_virtual_grouping)
                                {
                                memcpy(&group_totalSizeOfHeaders, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_totalSizeOfHeaders));
                                position += sizeof(group_totalSizeOfHeaders);
                                }

                            size_t group_numNodes;
                            memcpy(&group_numNodes, reinterpret_cast<char *>(dest.get()) + position, sizeof(size_t));
                            position += sizeof(size_t);
                            //assert(group_size <= s_max_number_nodes_in_group);

                            auto group = HFCPtr<SMNodeGroup>(new SMNodeGroup(getDataSourceAccount(), group_id, group_numNodes, group_totalSizeOfHeaders));
                            // NEEDS_WORK_SM : group datasource doesn't need to depend on type of grouping
                            group->SetDataSource(s_is_virtual_grouping ? m_pathToHeaders : m_pathToHeaders + L"g_", m_stream_store);
                            m_nodeHeaderGroups.push_back(group);

                            vector<uint64_t> nodeIds(group_numNodes);
                            memcpy(nodeIds.data(), reinterpret_cast<char *>(dest.get()) + position, group_numNodes*sizeof(uint64_t));
                            position += group_numNodes*sizeof(uint64_t);

                            group->GetHeader()->resize(group_numNodes);
                            transform(begin(nodeIds), end(nodeIds), begin(*group->GetHeader()), [](const uint64_t& nodeId)
                                {
                                return SMNodeHeader{ nodeId, 0, 0 };
                                });
						}

					}
				}
				else
				{
					Json::Reader	reader;
					Json::Value		masterHeader;
					wstringstream	ss;

					ss << m_path << L"MasterHeader.sscm";

					DataSourceURL dataSourceURL(ss.str());

					dataSource = initializeDataSource(dest, destSize);
					if (dataSource == nullptr)
						return 0;

					if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
						return 0;

					if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
						return 0;

					dataSource->close();

					headerSize = readSize;

					reader.parse(reinterpret_cast<char *>(dest.get()), reinterpret_cast<char *>(&(dest.get()[readSize])), masterHeader);

					indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
					indexHeader->m_balanced = masterHeader["balanced"].asBool();
					indexHeader->m_depth = masterHeader["depth"].asUInt();
                    indexHeader->m_isTerrain = masterHeader["isTerrain"].asBool();
                    assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

					auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
					indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

					if (masterHeader.isMember("singleFile"))
					{
						indexHeader->m_singleFile = masterHeader["singleFile"].asBool();
						HASSERT(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem
					}
				}

				return headerSize;
			}

			return 0;
		}


        virtual size_t LoadMasterHeader_Old(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
            {

            if (indexHeader != NULL)
                {

                if (m_use_node_header_grouping || s_stream_from_grouped_store)
                    {
                    wstringstream ss;
                    ss << m_path << L"../MasterHeaderWithGroups.bin";
                    auto filename = ss.str();
                    if (m_nodeHeaderGroups.empty())
                        {
                        char* masterHeaderBuffer = nullptr;
                        if (s_stream_from_disk)
                            {
                            BeFile file;
                            if (BeFileStatus::Success != OPEN_FILE(file, filename.c_str(), BeFileAccess::Read))
                                {
                                return 0;
                                }
                            file.GetSize(headerSize);
                            //bvector<Byte> masterHeaderBuffer(fileSize);
                            masterHeaderBuffer = new char[headerSize];
                            uint32_t bytes_read;
                            file.Read(masterHeaderBuffer, &bytes_read, (uint32_t)headerSize);
                            assert(bytes_read == headerSize);

                            file.Close();
                            }
                        else
                            {
                            m_stream_store.DownloadBlob(filename.c_str(), [indexHeader, &headerSize, &masterHeaderBuffer](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                                {
                                if (buffer.empty())
                                    {
                                    headerSize = 0;
                                    return;
                                    }
                                headerSize = (uint32_t)buffer.size();

                                masterHeaderBuffer = new char[headerSize];
                                memcpy(masterHeaderBuffer, buffer.data(), headerSize);
                                });
                            }

                        uint64_t position = 0;

                        uint32_t sizeOfOldMasterHeaderPart;
                        memcpy(&sizeOfOldMasterHeaderPart, masterHeaderBuffer + position, sizeof(sizeOfOldMasterHeaderPart));
                        position += sizeof(sizeOfOldMasterHeaderPart);
                        assert(sizeOfOldMasterHeaderPart == sizeof(SQLiteIndexHeader));

                        SQLiteIndexHeader oldMasterHeader;
                        memcpy(&oldMasterHeader, masterHeaderBuffer + position, sizeof(SQLiteIndexHeader));
                        position += sizeof(SQLiteIndexHeader);
                        indexHeader->m_SplitTreshold = oldMasterHeader.m_SplitTreshold;
                        indexHeader->m_balanced = oldMasterHeader.m_balanced;
                        indexHeader->m_depth = oldMasterHeader.m_depth;
                        indexHeader->m_isTerrain = oldMasterHeader.m_isTerrain;
                        indexHeader->m_singleFile = oldMasterHeader.m_singleFile;
                        assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

                        auto rootNodeBlockID = oldMasterHeader.m_rootNodeBlockID;
                        indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

                        memcpy(&s_is_virtual_grouping, masterHeaderBuffer + position, sizeof(s_is_virtual_grouping));
                        position += sizeof(s_is_virtual_grouping);

                        // Parse rest of file -- group information
                        while (position < headerSize)
                            {
                            size_t group_id;
                            memcpy(&group_id, masterHeaderBuffer + position, sizeof(group_id));
                            position += sizeof(group_id);

                            uint64_t group_totalSizeOfHeaders(0);
                            if (s_is_virtual_grouping)
                                {
                                memcpy(&group_totalSizeOfHeaders, masterHeaderBuffer + position, sizeof(group_totalSizeOfHeaders));
                                position += sizeof(group_totalSizeOfHeaders);
                                }

                            size_t group_numNodes;
                            memcpy(&group_numNodes, masterHeaderBuffer + position, sizeof(size_t));
                            position += sizeof(size_t);
                            //assert(group_size <= s_max_number_nodes_in_group);

                            auto group = HFCPtr<SMNodeGroup>(new SMNodeGroup(getDataSourceAccount(), group_id, group_numNodes, group_totalSizeOfHeaders));
                            // NEEDS_WORK_SM : group datasource doesn't need to depend on type of grouping
                            group->SetDataSource(s_is_virtual_grouping ? m_pathToHeaders : m_pathToHeaders + L"g_", m_stream_store);
                            m_nodeHeaderGroups.push_back(group);

                            vector<uint64_t> nodeIds(group_numNodes);
                            memcpy(nodeIds.data(), masterHeaderBuffer + position, group_numNodes*sizeof(uint64_t));
                            position += group_numNodes*sizeof(uint64_t);

                            group->GetHeader()->resize(group_numNodes);
                            transform(begin(nodeIds), end(nodeIds), begin(*group->GetHeader()), [](const uint64_t& nodeId)
                                {
                                return SMNodeHeader{ nodeId, 0, 0 };
                                });
                            }

                        delete[] masterHeaderBuffer;
                        }
                    }

                else if (s_stream_from_disk)
                    {
                    // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
                    BeFile file;
                    auto filename = (m_path + L"..\\MasterHeader.sscm").c_str();
                    if (BeFileStatus::Success != OPEN_FILE(file, filename, BeFileAccess::Read))//file.Open(filename, BeFileAccess::Read, BeFileSharing::None))
                        {
                        //assert(!"Local master header could not be found"); // possible during SM generation
                        return 0;
                        }
                    char inBuffer[100000];
                    uint32_t bytes_read = 0;
                    file.Read(inBuffer, &bytes_read, (uint32_t)headerSize);

                    Json::Reader reader;
                    Json::Value masterHeader;
                    reader.parse(&inBuffer[0], &inBuffer[bytes_read], masterHeader);

                    indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
                    indexHeader->m_balanced = masterHeader["balanced"].asBool();
                    indexHeader->m_depth = masterHeader["depth"].asUInt();
                    indexHeader->m_isTerrain = masterHeader["isTerrain"].asBool();
                    indexHeader->m_singleFile = masterHeader["singleFile"].asBool();
                    assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

                    auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
                    indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();
                    file.Close();

                    // Save in local stm file
                    //if (m_DTMFile != NULL)
                    //	return SMPointTaggedTileStore::StoreMasterHeader(indexHeader, headerSize);
                    }
                else {
                    auto blob_name = m_path + L"..//MasterHeader.sscm";
                    m_stream_store.DownloadBlob(blob_name.c_str(), [indexHeader, &headerSize](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                        {
                        if (buffer.empty())
                            {
                            headerSize = 0;
                            return;
                            }
                        headerSize = (uint32_t)buffer.size();
                        Json::Reader reader;
                        Json::Value masterHeader;
                        reader.parse(reinterpret_cast<const char*>(&buffer.front()), reinterpret_cast<const char*>(&buffer.back()), masterHeader);

                        indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
                        indexHeader->m_balanced = masterHeader["balanced"].asBool();
                        indexHeader->m_depth = masterHeader["depth"].asUInt();
                        indexHeader->m_isTerrain = masterHeader["isTerrain"].asBool();

                        auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
                        indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

                        });

                    }
                return headerSize;
                }

            return 0;
            }

        virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData)
            {
            assert(false); // Should not pass here
            return HPMBlockID(-1);
            }

        virtual HPMBlockID StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            HPRECONDITION(blockID.IsValid());

            auto blockIDConvert = ConvertBlockID(blockID);
            if (NULL != DataTypeArray && countData > 0)
                {
                wstringstream ss;
                ss << m_path << L"p_" << blockIDConvert << L".bin";
                auto filename = ss.str();
                BeFile file;
                auto fileOpened = OPEN_FILE(file, filename.c_str(), BeFileAccess::Write);
                if (BeFileStatus::Success != fileOpened)
                    {
                    auto fileCreated = file.Create(filename.c_str());
                    assert(BeFileStatus::Success == fileCreated);
                    fileOpened = fileCreated;
                    }
                assert(BeFileStatus::Success == fileOpened);

                HCDPacket uncompressedPacket, compressedPacket;
                size_t bufferSize = countData * sizeof(POINT);
                Byte* dataArrayTmp = new Byte[bufferSize];
                memcpy(dataArrayTmp, DataTypeArray, bufferSize);
                uncompressedPacket.SetBuffer(dataArrayTmp, bufferSize);
                uncompressedPacket.SetDataSize(bufferSize);
                WriteCompressedPacket(uncompressedPacket, compressedPacket);

                Byte* data = new Byte[compressedPacket.GetDataSize() + sizeof(uint32_t)];
                auto UncompressedSize = (uint32_t)uncompressedPacket.GetDataSize();
                reinterpret_cast<uint32_t&>(*data) = UncompressedSize;
                if (m_countInfo.count(blockIDConvert) > 0)
                    {
                    // must update data count
                    auto& points = this->m_countInfo[blockIDConvert];
                    points.resize(UncompressedSize);
                    memcpy(points.data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());
                    }

                memcpy(data + sizeof(uint32_t), compressedPacket.GetBufferAddress(), compressedPacket.GetDataSize());
                file.Write(NULL, data, (uint32_t)compressedPacket.GetDataSize() + sizeof(uint32_t));
                file.Close();
                delete[] data;
                delete[] dataArrayTmp;
                }
            return blockID;
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            if (IsValidID(blockID))
                return this->GetBlock(blockID).size() / sizeof(POINT);
            return 0;
            }

        virtual void   SerializeHeaderToBinary(const SMPointNodeHeader<EXTENT>* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize, uint32_t pi_pMaxDataSize = 3000) const
            {
            assert(po_pBinaryData == nullptr && po_pDataSize == 0);

            po_pBinaryData.reset(new Byte[3000]);

            const auto filtered = pi_pHeader->m_filtered;
            memcpy(po_pBinaryData.get() + po_pDataSize, &filtered, sizeof(filtered));
            po_pDataSize += sizeof(filtered);
            const auto parentBlockID = pi_pHeader->m_parentNodeID.IsValid() ? ConvertBlockID(pi_pHeader->m_parentNodeID) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &parentBlockID, sizeof(parentBlockID));
            po_pDataSize += sizeof(parentBlockID);
            const auto subNodeNoSplitID = pi_pHeader->m_SubNodeNoSplitID.IsValid() ? ConvertBlockID(pi_pHeader->m_SubNodeNoSplitID) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &subNodeNoSplitID, sizeof(subNodeNoSplitID));
            po_pDataSize += sizeof(subNodeNoSplitID);
            const auto level = pi_pHeader->m_level;
            memcpy(po_pBinaryData.get() + po_pDataSize, &level, sizeof(level));
            po_pDataSize += sizeof(level);
            const auto isBranched = pi_pHeader->m_IsBranched;
            memcpy(po_pBinaryData.get() + po_pDataSize, &isBranched, sizeof(isBranched));
            po_pDataSize += sizeof(isBranched);
            const auto isLeaf = pi_pHeader->m_IsLeaf;
            memcpy(po_pBinaryData.get() + po_pDataSize, &isLeaf, sizeof(isLeaf));
            po_pDataSize += sizeof(isLeaf);
            const auto splitThreshold = pi_pHeader->m_SplitTreshold;
            memcpy(po_pBinaryData.get() + po_pDataSize, &splitThreshold, sizeof(splitThreshold));
            po_pDataSize += sizeof(splitThreshold);
            const auto totalCount = pi_pHeader->m_totalCount;
            memcpy(po_pBinaryData.get() + po_pDataSize, &totalCount, sizeof(totalCount));
            po_pDataSize += sizeof(totalCount);
            const auto nodeCount = pi_pHeader->m_nodeCount;
            memcpy(po_pBinaryData.get() + po_pDataSize, &nodeCount, sizeof(nodeCount));
            po_pDataSize += sizeof(nodeCount);
            const auto arePoints3d = pi_pHeader->m_arePoints3d;
            memcpy(po_pBinaryData.get() + po_pDataSize, &arePoints3d, sizeof(arePoints3d));
            po_pDataSize += sizeof(arePoints3d);
            const auto isTextured = pi_pHeader->m_isTextured;
            memcpy(po_pBinaryData.get() + po_pDataSize, &isTextured, sizeof(isTextured));
            po_pDataSize += sizeof(isTextured);
            const auto nbFaceIndexes = pi_pHeader->m_nbFaceIndexes;
            memcpy(po_pBinaryData.get() + po_pDataSize, &nbFaceIndexes, sizeof(nbFaceIndexes));
            po_pDataSize += sizeof(nbFaceIndexes);
            const auto graphID = pi_pHeader->m_graphID.IsValid() ? ConvertBlockID(pi_pHeader->m_graphID) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &graphID, sizeof(graphID));
            po_pDataSize += sizeof(graphID);

            memcpy(po_pBinaryData.get() + po_pDataSize, &pi_pHeader->m_nodeExtent, 6*sizeof(double));
            po_pDataSize += 6*sizeof(double);

            const auto contentExtentDefined = pi_pHeader->m_contentExtentDefined;
            memcpy(po_pBinaryData.get() + po_pDataSize, &contentExtentDefined, sizeof(contentExtentDefined));
            po_pDataSize += sizeof(contentExtentDefined);
            if (contentExtentDefined)
                {
                memcpy(po_pBinaryData.get() + po_pDataSize, &pi_pHeader->m_contentExtent, 6 * sizeof(double));
                po_pDataSize += 6*sizeof(double);
                }

            /* Indice IDs */
            const auto idx = pi_pHeader->m_ptsIndiceID[0].IsValid() ? ConvertBlockID(pi_pHeader->m_ptsIndiceID[0]) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &idx, sizeof(idx));
            po_pDataSize += sizeof(idx);
            

            /* Mesh components and clips */
            const auto numberOfMeshComponents = pi_pHeader->m_numberOfMeshComponents;
            memcpy(po_pBinaryData.get() + po_pDataSize, &numberOfMeshComponents, sizeof(numberOfMeshComponents));
            po_pDataSize += sizeof(numberOfMeshComponents);
            for (size_t componentIdx = 0; componentIdx < pi_pHeader->m_numberOfMeshComponents; componentIdx++)
                {
                const auto component = pi_pHeader->m_meshComponents[componentIdx];
                memcpy(po_pBinaryData.get() + po_pDataSize, &component, sizeof(component));
                po_pDataSize += sizeof(component);
                }

            const auto nbClipSetsIDs = (uint32_t)pi_pHeader->m_clipSetsID.size();
            memcpy(po_pBinaryData.get() + po_pDataSize, &nbClipSetsIDs, sizeof(nbClipSetsIDs));
            po_pDataSize += sizeof(nbClipSetsIDs);
            for (size_t i = 0; i < nbClipSetsIDs; ++i)
                {
                const auto clip = ConvertNeighborID(pi_pHeader->m_clipSetsID[i]);
                memcpy(po_pBinaryData.get() + po_pDataSize, &clip, sizeof(clip));
                po_pDataSize += sizeof(clip);
                }

            /* Children and Neighbors */
            const auto nbChildren = isLeaf || (!isBranched  && !pi_pHeader->m_SubNodeNoSplitID.IsValid()) ? 0 : (!isBranched ? 1 : pi_pHeader->m_numberOfSubNodesOnSplit);
            memcpy(po_pBinaryData.get() + po_pDataSize, &nbChildren, sizeof(nbChildren));
            po_pDataSize += sizeof(nbChildren);
            for (size_t childInd = 0; childInd < nbChildren; childInd++)
                {
                const auto id = ConvertChildID(pi_pHeader->m_apSubNodeID[childInd]);
                memcpy(po_pBinaryData.get() + po_pDataSize, &id, sizeof(id));
                po_pDataSize += sizeof(id);
                }

            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {
                const auto numNeighbors = pi_pHeader->m_apNeighborNodeID[neighborPosInd].size();
                memcpy(po_pBinaryData.get() + po_pDataSize, &numNeighbors, sizeof(numNeighbors));
                po_pDataSize += sizeof(numNeighbors);
                for (size_t neighborInd = 0; neighborInd < numNeighbors; neighborInd++)
                    {
                    const auto nodeId = ConvertNeighborID(pi_pHeader->m_apNeighborNodeID[neighborPosInd][neighborInd]);
                    memcpy(po_pBinaryData.get() + po_pDataSize, &nodeId, sizeof(nodeId));
                    po_pDataSize += sizeof(nodeId);
                    }
                }
            }

        void SerializeHeaderToJSON(const SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& block)
            {
            block["id"] = ConvertBlockID(blockID);
            block["resolution"] = (IDTMFile::NodeID)header->m_level;
            block["filtered"] = header->m_filtered;
            block["parentID"] = header->m_parentNodeID.IsValid() ? ConvertBlockID(header->m_parentNodeID) : IDTMFile::GetNullNodeID();
            block["isLeaf"] = header->m_IsLeaf;
            block["isBranched"] = header->m_IsBranched;
            block["splitThreshold"] = header->m_SplitTreshold;

            size_t nbChildren = header->m_IsLeaf || (!header->m_IsBranched  && !header->m_SubNodeNoSplitID.IsValid()) ? 0 : (!header->m_IsBranched ? 1 : header->m_numberOfSubNodesOnSplit);

            block["nbChildren"] = nbChildren;

            auto& children = block["children"];

            if (nbChildren > 1)
                {
                for (size_t childInd = 0; childInd < nbChildren; childInd++)
                    {
                    Json::Value& child = childInd >= children.size() ? children.append(Json::Value()) : children[(int)childInd];
                    child["index"] = (uint8_t)childInd;
                    child["id"] = header->m_apSubNodeID[childInd].IsValid() ? ConvertChildID(header->m_apSubNodeID[childInd]) : IDTMFile::GetNullNodeID();
                    }
                }
            else if (nbChildren == 1)
                {
                Json::Value& child = children.empty() ? children.append(Json::Value()) : children[0];
                child["index"] = 0;
                child["id"] = header->m_SubNodeNoSplitID.IsValid() ? ConvertChildID(header->m_SubNodeNoSplitID) : ConvertChildID(header->m_apSubNodeID[0]);
                }

            auto& neighbors = block["neighbors"];
            int neighborInfoInd = 0;
            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {
                for (size_t neighborInd = 0; neighborInd < header->m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
                    {
                    Json::Value& neighbor = (uint32_t)neighborInfoInd >= neighbors.size() ? neighbors.append(Json::Value()) : neighbors[(uint32_t)neighborInfoInd];
                    neighbor["nodePos"] = (uint8_t)neighborPosInd;
                    neighbor["nodeId"] = ConvertNeighborID(header->m_apNeighborNodeID[neighborPosInd][neighborInd]);
                    neighborInfoInd++;
                    }
                }

            auto& extent = block["nodeExtent"];

            extent["xMin"] = ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent);
            extent["yMin"] = ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent);
            extent["zMin"] = ExtentOp<EXTENT>::GetZMin(header->m_nodeExtent);
            extent["xMax"] = ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent);
            extent["yMax"] = ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent);
            extent["zMax"] = ExtentOp<EXTENT>::GetZMax(header->m_nodeExtent);

            if (header->m_contentExtentDefined)
                {
                block["contentExtentDefined"] = true;
                auto& contentExtent = block["contentExtent"];
                contentExtent["xMin"] = ExtentOp<EXTENT>::GetXMin(header->m_contentExtent);
                contentExtent["yMin"] = ExtentOp<EXTENT>::GetYMin(header->m_contentExtent);
                contentExtent["zMin"] = ExtentOp<EXTENT>::GetZMin(header->m_contentExtent);
                contentExtent["xMax"] = ExtentOp<EXTENT>::GetXMax(header->m_contentExtent);
                contentExtent["yMax"] = ExtentOp<EXTENT>::GetYMax(header->m_contentExtent);
                contentExtent["zMax"] = ExtentOp<EXTENT>::GetZMax(header->m_contentExtent);
                }
            else
                {
                block["contentExtentDefined"] = false;
                }


            block["totalCount"] = header->m_totalCount;
            block["nodeCount"] = header->m_nodeCount;
            block["arePoints3d"] = header->m_arePoints3d;

            /*

            //why was this commented?
            // assert(header->m_3dPointsDescBins.size() <= USHORT_MAX);
            // m_indexHandler->SetNb3dPointsBins(ConvertBlockID(blockID), header->m_3dPointsDescBins.size());

            */

            block["nbFaceIndexes"] = header->m_nbFaceIndexes;
            block["graphID"] = header->m_graphID.IsValid() ? ConvertBlockID(header->m_graphID) : IDTMFile::GetNullNodeID();
            block["nbIndiceID"] = (int)header->m_ptsIndiceID.size();

            auto& indiceID = block["indiceID"];
            for (size_t i = 0; i < header->m_ptsIndiceID.size(); i++)
                {
                Json::Value& indice = (uint32_t)i >= indiceID.size() ? indiceID.append(Json::Value()) : indiceID[(uint32_t)i];
                indice = header->m_ptsIndiceID[i].IsValid() ? ConvertBlockID(header->m_ptsIndiceID[i]) : IDTMFile::GetNullNodeID();
                }

            if (header->m_isTextured /*&& !header->m_textureID.empty() && IsValidID(header->m_textureID[0])*/)
                {
                block["areTextured"] = true;
                /*block["nbTextureIDs"] = (int)header->m_textureID.size();
                auto& textureIDs = block["textureIDs"];
                for (size_t i = 0; i < header->m_textureID.size(); i++)
                    {
                    auto convertedID = ConvertBlockID(header->m_textureID[i]);
                    if (convertedID != IDTMFile::GetNullNodeID())
                        {
                        Json::Value& textureID = (uint32_t)i >= textureIDs.size() ? textureIDs.append(Json::Value()) : textureIDs[(uint32_t)i];
                        textureID = header->m_textureID[i].IsValid() ? convertedID : IDTMFile::GetNullNodeID();
                        }
                    }*/
                block["uvID"] = header->m_uvID.IsValid() ? ConvertBlockID(header->m_uvID) : IDTMFile::GetNullNodeID();

                block["nbUVIDs"] = (int)header->m_uvsIndicesID.size();
                auto& uvIndiceIDs = block["uvIndiceIDs"];
                for (size_t i = 0; i < header->m_uvsIndicesID.size(); i++)
                    {
                    Json::Value& uvIndice = (uint32_t)i >= uvIndiceIDs.size() ? uvIndiceIDs.append(Json::Value()) : uvIndiceIDs[(uint32_t)i];
                    uvIndice = header->m_uvsIndicesID[i].IsValid() ? ConvertBlockID(header->m_uvsIndicesID[i]) : IDTMFile::GetNullNodeID();
                    }
                }
            else {
                block["areTextured"] = false;
                }

            block["numberOfMeshComponents"] = header->m_numberOfMeshComponents;
            auto& meshComponents = block["meshComponents"];
            for (size_t componentIdx = 0; componentIdx < header->m_numberOfMeshComponents; componentIdx++)
                {
                auto& component = (uint32_t)componentIdx >= meshComponents.size() ? meshComponents.append(Json::Value()) : meshComponents[(uint32_t)componentIdx];
                component = header->m_meshComponents[componentIdx];
                }

            if (header->m_clipSetsID.size() > 0)
                {
                auto& clipSetsID = block["clipSetsID"];
                for (size_t i = 0; i < header->m_clipSetsID.size(); ++i)
                    {
                    auto& clip = (uint32_t)i >= clipSetsID.size() ? clipSetsID.append(Json::Value()) : clipSetsID[(uint32_t)i];
                    clip = ConvertNeighborID(header->m_clipSetsID[i]);
                    }
                }
            //  else
            block["nbClipSets"] = (uint32_t)header->m_clipSetsID.size();
            }

        virtual size_t StoreHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            uint32_t headerSize = 0;
            std::unique_ptr<Byte> headerData = nullptr;
            SerializeHeaderToBinary(header, headerData, headerSize);
            //SerializeHeaderToJSON(header, blockID, block);

            wstringstream ss;
            ss << m_pathToHeaders << L"n_" << ConvertBlockID(blockID) << L".bin";

            auto filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, filename.c_str(), BeFileAccess::Write) || BeFileStatus::Success == file.Create(filename.c_str()))
                {
                //    Json::StyledWriter writer;
                //    auto buffer = writer.write(object);
                //    buffer_size = buffer.size();
                //    file.Write(NULL, buffer.c_str(), buffer_size);
                file.Write(NULL, headerData.get(), headerSize);
                }
            else
                {
                assert(!"Problem opening/creating header file");
                }
            file.Close();

            return 1;
            }

        virtual size_t LoadHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            if (s_stream_from_grouped_store)
                {
                auto group = this->GetGroup(blockID);
                auto node_header = group->GetNodeHeader(ConvertBlockID(blockID));
                ReadNodeHeaderFromBinary(header, group->GetRawHeaders(node_header.offset), node_header.size);
                }
            else {
                //auto nodeHeader = this->GetNodeHeaderJSON(blockID);
                //ReadNodeHeaderFromJSON(header, nodeHeader);
                uint64_t headerSize = 0;
                std::unique_ptr<Byte> headerData = nullptr;
                this->GetNodeHeaderBinary(blockID, headerData, headerSize);
                ReadNodeHeaderFromBinary(header, headerData.get(), headerSize);
                }
            return 1;
            }

        virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            auto& block = this->GetBlock(blockID);
            assert(block.size() <= maxCountData * sizeof(POINT));
            memcpy(DataTypeArray, block.data(), block.size());

            return block.size();
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return true;
            }

    private:

        WString m_path;
        WString m_pathToHeaders;
        bool m_use_node_header_grouping;
        // NEEDS_WORK_SM_STREAMING: should only have one stream store for all data types
        WString m_storage_connection_string;
        scalable_mesh::azure::Storage m_stream_store;
        bvector<HFCPtr<SMNodeGroup>> m_nodeHeaderGroups;
        mutable std::map<IDTMFile::NodeID, PointBlock> m_countInfo;
        std::mutex headerLock;
    };
