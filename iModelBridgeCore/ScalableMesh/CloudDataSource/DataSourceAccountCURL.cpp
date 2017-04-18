#include "stdafx.h"
#include <assert.h>
#include <algorithm>
#include <sstream>
#include <curl/curl.h>
#include <openssl/crypto.h>
#include <locale>
#include <Bentley/BeFileName.h>
#include "include/DataSourceAccountCURL.h"
#include "include/DataSourceCURL.h"

OpenSSLMutexes* OpenSSLMutexes::s_instance = nullptr;

OpenSSLMutexes::OpenSSLMutexes(size_t numMutexes)
    {
    m_mutexes = new std::mutex[numMutexes * sizeof(std::mutex)];
    }

OpenSSLMutexes::~OpenSSLMutexes()
    {
    delete[] m_mutexes;
    }

std::mutex * OpenSSLMutexes::GetMutexes()
    {
    return m_mutexes;
    }

OpenSSLMutexes * OpenSSLMutexes::CreateInstance(const size_t & numMutexes)
    {
    if (!s_instance)
        {
        s_instance = new OpenSSLMutexes(numMutexes);
        }
    return s_instance;
    }

OpenSSLMutexes * OpenSSLMutexes::Instance()
    {
    assert(nullptr != s_instance); // instance must be created first!
    return s_instance;
    }

DataSourceAccountCURL::DataSourceAccountCURL(const ServiceName & name, const AccountIdentifier & identifier, const AccountKey & key)
{
    setAccount(name, identifier, key);
                                                            // Default size is set by Service on creation
    setDefaultSegmentSize(0);

                                                            // Multi-threaded segmented transfers used for Azure, so initialize it
    getTransferScheduler()->initializeTransferTasks(getDefaultNumTransferTasks());

    curl_global_init(CURL_GLOBAL_ALL);

    OpenSSLMutexes::CreateInstance(CRYPTO_num_locks());

    CRYPTO_set_locking_callback(CURLHandle::OpenSSLLockingFunction);

    }


DataSource * DataSourceAccountCURL::createDataSource(void)
{
                                                            // NOTE: This method is for internal use only, don't call this directly.
    DataSourceCURL *   dataSourceCURL;
                                                            // Create a new DataSourceAzure
    dataSourceCURL = new DataSourceCURL(this);
    if (dataSourceCURL == nullptr)
        return nullptr;
                                                            // Set the timeout from the account's default (which comes from the Service's default)
    dataSourceCURL->setTimeout(this->getDefaultTimeout());
                                                            // Set the segment size from the account's default (which comes from the Service's default)
    dataSourceCURL->setSegmentSize(this->getDefaultSegmentSize());

    return dataSourceCURL;
}


DataSourceStatus DataSourceAccountCURL::destroyDataSource(DataSource *dataSource)
{
    if (dataSource)
    {
        delete dataSource;

        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}


DataSourceAccountCURL::~DataSourceAccountCURL(void)
    {
    curl_global_cleanup();
    }

void DataSourceAccountCURL::setDefaultSegmentSize(DataSourceBuffer::BufferSize size)
{
    defaultSegmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceAccountCURL::getDefaultSegmentSize(void)
{
    return defaultSegmentSize;
}

void DataSourceAccountCURL::setDefaultTimeout(DataSourceBuffer::Timeout time)
    {
    defaultTimeout = time;
    }

DataSourceBuffer::Timeout DataSourceAccountCURL::getDefaultTimeout(void)
    {
    return defaultTimeout;
    }

DataSourceStatus DataSourceAccountCURL::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
{
    if (account.length() == 0 )
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
                                                            // Set details in base class
    DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceCURL"), account, identifier, key);

    return DataSourceStatus();
}

void DataSourceAccountCURL::setPrefixPath(const DataSourceURL &prefix)
    {
    //// Default is local or network files
    //isLocalOrNetworkAccount = true;
    DataSourceAccount::setPrefixPath(prefix);
    }


DataSourceStatus DataSourceAccountCURL::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;

    dataSource.getURL(url);

    return downloadBlobSync(url, dest, readSize, destSize);
}

DataSourceStatus DataSourceAccountCURL::downloadBlobSync(DataSourceURL &url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size)
    {
    if (isLocalOrNetworkAccount)
        {
        url = L"file:///" + url;
        }
    struct CURLHandle::CURLDataMemoryBuffer buffer;

    buffer.data = dest;
    buffer.size = 0;

    Utf8String utf8URL (url.c_str());

    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();

    CURL* curl = curl_handle->get();
    curl_easy_setopt(curl, CURLOPT_URL, utf8URL.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLHandle::CURLWriteDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);

#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, DataSourceAccountCURL::CURLHandle::CURLWriteHeaderCallback);
    struct CURLHandle::CURLDataResponseHeader response_header;
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
#endif
    auto res = curl_easy_perform(curl);
    if (CURLE_OK != res)
        {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        assert(!"cURL error, download failed");
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
        }

#ifndef NDEBUG
    if (!response_header.data.empty() && response_header.data["HTTP"] != "1.1 200 OK")
        {
        assert(!"HTTP error, download failed or resource not found");
        return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
        }
    if (!response_header.data.empty()) response_header.data.clear();
#endif

    curl_handle->free_header_list();

    assert(buffer.size <= size);
    readSize = buffer.size;
    (void)size;

    return DataSourceStatus();
    }

DataSourceStatus DataSourceAccountCURL::uploadBlobSync(DataSourceURL &url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    if (isLocalOrNetworkAccount)
        {
#ifndef NDEBUG
        BeFileName file(url.c_str());
#ifndef VANCOUVER_API
        assert(false == file.DoesPathExist()); // file should not exist
#else
        assert(false == BeFileName::DoesPathExist(url.c_str()));
#endif
#endif
        url = L"file:///" + url;
        }

    struct CURLHandle::CURLDataMemoryBuffer buffer;
    buffer.data = source;
    buffer.size = size;
    struct CURLHandle::CURLDataResponseHeader response_header;

    Utf8String utf8URL = Utf8String(url.c_str());
    std::string contentLength = "Content-Length " + std::to_string(size);
    std::string contentDisposition = "Content-Disposition: attachment; filename=\"";
    contentDisposition += Utf8String(filename.c_str()).c_str();
    contentDisposition += "\"";

    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();
    CURL* curl = curl_handle->get();

    curl_handle->add_item_to_header("Content-Type: text/plain");
    curl_handle->add_item_to_header(contentLength.c_str());
    curl_handle->add_item_to_header(contentDisposition.c_str());


    curl_easy_setopt(curl, CURLOPT_URL,        utf8URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_handle->get_headers());
    //curl_easy_setopt(curl, CURLOPT_HEADEROPT,  CURLHEADER_SEPARATE); // This is for connections with a proxy
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);        // &&RB TODO : Ask Francis.Boily about his server certificate
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,           1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,    CURLHandle::CURLDummyWriteDataCallback); // No output to console
    curl_easy_setopt(curl, CURLOPT_READFUNCTION,     CURLHandle::CURLReadDataCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION,   CURLHandle::CURLWriteHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA,         buffer);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA,       &response_header);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, size);

    /* put it! */
    CURLcode res = curl_easy_perform(curl);

    /* check for errors */
    if (CURLE_OK != res)
        {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
        }

    //curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L);
    //DataSourceBuffer::BufferData * download_buffer = new DataSourceBuffer::BufferData[size];
    //DataSourceBuffer::BufferSize readSize;
    //DataSourceAccountCURL::downloadBlobSync(url, download_buffer, readSize, size);

    curl_handle->free_header_list();

    return DataSourceStatus();
    }

DataSourceStatus DataSourceAccountCURL::uploadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL    url;
    dataSource.getURL(url);

    return uploadBlobSync(url, dataSource.getSubPath().c_str(), source, size);
    }

size_t DataSourceAccountCURL::CURLHandle::CURLWriteHeaderCallback(void * contents, size_t size, size_t nmemb, void * userp)
    {
    if (userp == nullptr) return 0;

    struct CURLDataResponseHeader *header = (struct CURLDataResponseHeader *)userp;

    std::istringstream resp((char*)contents);
    std::string line;
    std::getline(resp, line);
    line.pop_back();
    if (!line.empty())
        {
        std::string::size_type index = line.find(':', 0);
        if (index != std::string::npos)
            {
            header->data.insert(std::make_pair(line.substr(0, index), line.substr(index + 2)));
            }
        else if ((index = line.find('/', 0)) != std::string::npos)
            {
            header->data.insert(std::make_pair(line.substr(0, index), line.substr(index + 1)));
            }
        }

    return size * nmemb;
    }

size_t DataSourceAccountCURL::CURLHandle::CURLWriteDataCallback(void * contents, size_t size, size_t nmemb, void * userp)
    {
    size_t realsize = size * nmemb;
    struct CURLDataMemoryBuffer *mem = (struct CURLDataMemoryBuffer *)userp;
    
    //assert(mem->memory->capacity() >= mem->memory->size() + realsize);
    
    //    mem->memory->assign((Byte*)contents, (Byte*)contents + realsize);
    //mem->memory->insert(mem->memory->end(), (uint8_t*)contents, (uint8_t*)contents + realsize);
    memcpy(&mem->data[0], (uint8_t*)contents, realsize);
    mem->data += realsize;
    mem->size += realsize;
    
    return realsize;
    }

size_t DataSourceAccountCURL::CURLHandle::CURLDummyWriteDataCallback(void * , size_t size, size_t nmemb, void * )
    {
    size_t realsize = size * nmemb;

    return realsize;
    }

size_t DataSourceAccountCURL::CURLHandle::CURLReadDataCallback(char * bufptr, size_t size, size_t nitems, void * userp)
    {
    struct CURLDataMemoryBuffer *mem = (struct CURLDataMemoryBuffer *)userp;
    size_t sizeToRead = std::min(mem->size, size * nitems);
    if (sizeToRead < 1)
        return 0;
    if (mem->size)
        {
        memcpy(bufptr, &mem->data[0], sizeToRead);
        mem->data += sizeToRead;
        mem->size -= sizeToRead;

        return sizeToRead;
        }
    return 0;
    }

void DataSourceAccountCURL::CURLHandle::OpenSSLLockingFunction(int mode, int n, const char * /*file*/, int /*line*/)
    {
    auto mutexes = OpenSSLMutexes::Instance()->GetMutexes();
    if (mode & CRYPTO_LOCK)
        mutexes[n].lock();
    else
        mutexes[n].unlock();
    }

DataSourceAccountCURL::CURLHandle* DataSourceAccountCURL::CURLHandleManager::getOrCreateCURLHandle(const HandleName & name, bool * created)
    {
    CURLHandle *    curl_handle = nullptr;

    // Attempt to get the named CURL handle
    curl_handle = Manager<DataSourceAccountCURL::CURLHandle, true>::get(name);
    if (curl_handle)
        {
        // If requested, flag that the DataSource existed and was not created
        if (created)
            *created = false;
        // Return the found DataSource
        assert(curl_handle != nullptr);
        return curl_handle;
        }

    // If requested, flag that the DataSource was created
    if (created)
        *created = true;

    // Otherwise, create it
    return createCURLHandle(name);
    }

DataSourceAccountCURL::CURLHandle * DataSourceAccountCURL::CURLHandleManager::getOrCreateThreadCURLHandle(bool * created)
    {
    std::wstringstream      name;
    DataSourceAccountCURL::CURLHandleManager::HandleName        handleName;

    // Get thread ID and use as CURL name
    std::thread::id threadID = std::this_thread::get_id();
    name << threadID;

    handleName = name.str();

    return this->getOrCreateCURLHandle(handleName, created);
    }

DataSourceAccountCURL::CURLHandle * DataSourceAccountCURL::CURLHandleManager::createCURLHandle(const HandleName & name)
    {
    CURL* curl = curl_easy_init();

    //curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE); // Only for proxy servers
    //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, DataSourceAccountCURL::CURLHandle::CURLWriteHeaderCallback);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0/*1*/);  // &&RB TODO : Ask Francis.Boily about his server certificate
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0/*1*/);  // At some point we will have a valid CONNECT certificate and we'll need to reactivate OpenSSL
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    //curl_easy_setopt(curl, CURLOPT_STDERR, std::cout);
    CURLHandle* curl_handle = new CURLHandle(curl);
    if (Manager<DataSourceAccountCURL::CURLHandle, true>::create(name, curl_handle) == NULL)
        {
        delete curl_handle;
        return nullptr;
        }

    return curl_handle;
    }
