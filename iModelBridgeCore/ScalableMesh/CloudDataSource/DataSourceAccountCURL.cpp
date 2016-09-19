#include "stdafx.h"
#include <assert.h>
#include <algorithm>
#include <sstream>
#include <curl/curl.h>
#include <openssl/crypto.h>
#include <locale>
#include <codecvt>
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
    getTransferScheduler().initializeTransferTasks(getDefaultNumTransferTasks());

    curl_global_init(CURL_GLOBAL_ALL);

    OpenSSLMutexes::CreateInstance(CRYPTO_num_locks());

    CRYPTO_set_locking_callback(DataSourceAccountCURL::OpenSSLLockingFunction);

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
    DataSourceURL url(prefix);
    // &&RB TODO

    DataSourceAccount::setPrefixPath(url);
    }


DataSourceStatus DataSourceAccountCURL::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;

    dataSource.getURL(url);

    return downloadBlobSync(url, dest, readSize, destSize);
}

DataSourceStatus DataSourceAccountCURL::downloadBlobSync(DataSourceURL &url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size)
{
    try
    {
    struct CURLDataMemoryBuffer buffer;
    //struct CURLDataResponseHeader response_header;

    buffer.data = dest;
    buffer.size = 0;

    std::string utf8URL = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(url);

    CURL* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();

    curl_easy_setopt(curl_handle, CURLOPT_URL, utf8URL.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&buffer);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, this->CURLWriteDataCallback);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &response_header); // &&RB TODO : check for valid response header??

    /* get it! */
    CURLcode res = curl_easy_perform(curl_handle);

    /* check for errors */
    if (CURLE_OK != res)
        {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        throw;
        }

    assert(buffer.size <= size);
    readSize = buffer.size;
    (void)size;
    }
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
    }

    return DataSourceStatus();
}

DataSourceStatus DataSourceAccountCURL::uploadBlobSync(DataSourceURL &url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    try
        {
        CURL *curl_handle;
        curl_handle = curl_easy_init();

        struct CURLDataMemoryBuffer buffer;
        buffer.data = source;
        buffer.size = size;

        struct CURLDataResponseHeader header;

        std::string utf8URL = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(url);

        std::string contentLength = "Content-Length " + std::to_string(size);
        std::string contentDisposition = "Content-Disposition: attachment; filename=\"";
        contentDisposition += std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(filename);
        contentDisposition += "\"";


        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: text/plain");
        headers = curl_slist_append(headers, contentLength.c_str());
        headers = curl_slist_append(headers, contentDisposition.c_str());

        curl_easy_setopt(curl_handle, CURLOPT_URL, utf8URL.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl_handle, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0/*1*/);        // &&RB TODO : Ask Francis.Boily about his server certificate
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0/*1*/);
        curl_easy_setopt(curl_handle, CURLOPT_CAINFO, this->getAccountSSLCertificatePath().c_str());
        curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, CURLDummyWriteDataCallback);
        curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, CURLReadDataCallback);
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, CURLWriteHeaderCallback);
        curl_easy_setopt(curl_handle, CURLOPT_READDATA, buffer);
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &header);
        curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, size);

        /* put it! */
        CURLcode res = curl_easy_perform(curl_handle);

        /* check for errors */
        if (CURLE_OK != res)
            {
            //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            throw;
            }

        curl_easy_cleanup(curl_handle);
        }
    catch (...)
        {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
        }
    //DataSourceBuffer::BufferData * download_buffer = new DataSourceBuffer::BufferData[size];
    //DataSourceBuffer::BufferSize readSize;
    //downloadBlobSync(url, download_buffer, readSize, size);

    return DataSourceStatus();
    }

DataSourceStatus DataSourceAccountCURL::uploadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL    url;
    dataSource.getURL(url);

    return uploadBlobSync(url, dataSource.getSubPath().c_str(), source, size);
    }

size_t DataSourceAccountCURL::CURLWriteHeaderCallback(void * contents, size_t size, size_t nmemb, void * userp)
    {
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

size_t DataSourceAccountCURL::CURLWriteDataCallback(void * contents, size_t size, size_t nmemb, void * userp)
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

size_t DataSourceAccountCURL::CURLDummyWriteDataCallback(void * , size_t size, size_t nmemb, void * )
    {
    size_t realsize = size * nmemb;

    return realsize;
    }

size_t DataSourceAccountCURL::CURLReadDataCallback(char * bufptr, size_t size, size_t nitems, void * userp)
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

void DataSourceAccountCURL::OpenSSLLockingFunction(int mode, int n, const char * /*file*/, int /*line*/)
    {
    auto mutexes = OpenSSLMutexes::Instance()->GetMutexes();
    if (mode & CRYPTO_LOCK)
        mutexes[n].lock();
    else
        mutexes[n].unlock();
    }

CURL * DataSourceAccountCURL::CURLHandleManager::getOrCreateCURLHandle(const HandleName & name, bool * created)
    {
    CURL **    curl_handle = nullptr;

    // Attempt to get the named CURL handle
    curl_handle = Manager<CURL*>::get(name);
    if (curl_handle)
        {
        // If requested, flag that the DataSource existed and was not created
        if (created)
            *created = false;
        // Return the found DataSource
        assert(curl_handle != nullptr);
        return *curl_handle;
        }

    // If requested, flag that the DataSource was created
    if (created)
        *created = true;

    // Otherwise, create it
    return createCURLHandle(name);
    }

CURL * DataSourceAccountCURL::CURLHandleManager::getOrCreateThreadCURLHandle(bool * created)
    {
    std::wstringstream      name;
    DataSourceAccountCURL::CURLHandleManager::HandleName        handleName;

    // Get thread ID and use as CURL name
    std::thread::id threadID = std::this_thread::get_id();
    name << threadID;

    handleName = name.str();

    return this->getOrCreateCURLHandle(handleName, created);
    }

CURL * DataSourceAccountCURL::CURLHandleManager::createCURLHandle(const HandleName & name)
    {
    CURL* curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, CURLWriteHeaderCallback);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0/*1*/);  // &&RB TODO : Ask Francis.Boily about his server certificate
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0/*1*/);  // At some point we will have a valid CONNECT certificate and we'll need to reactivate OpenSSL
    //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
    //curl_easy_setopt(curl_handle, CURLOPT_STDERR, std::cout);
    if (Manager<CURL*>::create(name, new CURL*(curl_handle)) == NULL)
        {
        return nullptr;
        }

    return curl_handle;
    }
