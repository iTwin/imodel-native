/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
#include <Logging/bentleylogging.h>


#ifdef LOG_CURL
#include <Bentley/BeFile.h>
#endif

#define DATASOURCEACCOUNTCURLLOGNAME L"ScalableMesh::CloudDataSourceAccountCURL"
#define DATASOURCEACCOUNTCURL_LOG (*NativeLogging::LoggingManager::GetLogger(DATASOURCEACCOUNTCURLLOGNAME))

#if _WIN32
#pragma warning(disable:4840)
#endif


#ifdef VANCOUVER_API
#define ISURL(filename) BeFileName::IsUrl(filename)
#else
#define ISURL(filename) NULL != filename && (0 == wcsncmp(L"http:", filename, 5) || 0 == wcsncmp(L"https:", filename, 6))
#endif
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

DataSourceAccountCURL::DataSourceAccountCURL(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
{
    setAccount(account, identifier, key);
                                                            // Default size is set by Service on creation
    setDefaultSegmentSize(0);

                                                            // Multi-threaded segmented transfers used for Azure, so initialize it
    getTransferScheduler()->initializeTransferTasks(getDefaultNumTransferTasks());

    curl_global_init(CURL_GLOBAL_ALL);

    OpenSSLMutexes::CreateInstance(CRYPTO_num_locks());

    CRYPTO_set_locking_callback(CURLHandle::OpenSSLLockingFunction);
    }

void DataSourceAccountCURL::setProxy(const Utf8String& proxyUserIn, const Utf8String& proxyPasswordIn, const Utf8String& proxyServerUrlIn)
    {
    proxyUser = proxyUserIn;
    proxyPassword = proxyPasswordIn;
    proxyServerUrl = proxyServerUrlIn;
    }

void DataSourceAccountCURL::setCertificateAuthoritiesUrl(const Utf8String& certificateAuthoritiesUrlIn)
    {
    certificateAuthoritiesUrl = certificateAuthoritiesUrlIn;
    }

DataSource * DataSourceAccountCURL::createDataSource(const SessionName &session)
{
                                                            // NOTE: This method is for internal use only, don't call this directly.
    DataSourceCURL *   dataSourceCURL;
                                                            // Create a new DataSourceAzure
    dataSourceCURL = new DataSourceCURL(this, session);
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
    DataSourceAccount::setPrefixPath(prefix);
    }


DataSourceStatus DataSourceAccountCURL::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;

    dataSource.getURL(url);

    return downloadBlobSync(url, dest, readSize, destSize, dataSource.getSessionName());
}

DataSourceStatus DataSourceAccountCURL::downloadBlobSync(DataSourceURL &url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session)
    {
    DataSourceStatus status;
    struct CURLHandle::CURLDataMemoryBuffer buffer;
    struct CURLHandle::CURLDataResponseHeader response_header;

    buffer.data.raw_data = dest;
    buffer.size = 0;
    buffer.max_size = size;

    Utf8String utf8URL (url.c_str());

    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();

    CURL* curl = curl_handle->get();
    curl_easy_setopt(curl, CURLOPT_URL, utf8URL.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L); // Make sure the upload flag is set to false
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLHandle::CURLWriteDataCallbackRaw);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, DataSourceAccountCURL::CURLHandle::CURLWriteHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);        // &&RB TODO : Ask Francis.Boily about his server certificate
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    setupCertificateAuthorities(curl);
    setupProxyToCurl(curl);

    auto res = curl_easy_perform(curl);
    if (CURLE_OK != res)
        {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        //assert(!"cURL error, download failed");
        status = DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
        }

    if (!response_header.data.empty() && response_header.data["HTTP"] != "1.1 200 OK")
        {
        //assert(!"HTTP error, download failed or resource not found");
        status = DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
        }

#ifdef LOG_CURL
    {
    BeFile file;
    uint32_t NbCharsWritten = 0;
    if (BeFileStatus::Success == file.Open(L"C:\\cds_log.txt", BeFileAccess::Write, BeFileSharing::None) || BeFileStatus::Success == file.Create(L"C:\\cds_log.txt"))
        {
        utf8URL += "\r\n";
        file.Write(&NbCharsWritten, utf8URL.c_str(), (uint32_t)utf8URL.size());
        char message[10000];
        sprintf(message, "Date: %s\r\nServer: %s\r\ncurl_easy_perform() result message: %s\r\nHTTP result code: %s\r\n",
            response_header.data["Date"].c_str(), response_header.data["Server"].c_str(), curl_easy_strerror(res), response_header.data["HTTP"].c_str());
        std::string curl_message(message);
        file.Write(&NbCharsWritten, curl_message.c_str(), (uint32_t)curl_message.size());
        }
    }
#endif

    if (!response_header.data.empty()) response_header.data.clear();

    curl_handle->free_header_list();

    if (status.isOK())
        {
        readSize = buffer.size;
        (void)size;
        }

    return status;
    }

DataSourceStatus DataSourceAccountCURL::downloadBlobSync(DataSourceURL &url, DataSourceBuffer* vector, const DataSource::SessionName &session)
    {
    DataSourceStatus status;
    struct CURLHandle::CURLDataMemoryBuffer buffer;
    struct CURLHandle::CURLDataResponseHeader response_header;

    buffer.data.vector = vector;
    buffer.size = 0;

    Utf8String utf8URL(url.c_str());

    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();

    CURL* curl = curl_handle->get();
    curl_easy_setopt(curl, CURLOPT_URL, utf8URL.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 0L); // Make sure the upload flag is set to false
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLHandle::CURLWriteDataCallbackVector);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 524288L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, DataSourceAccountCURL::CURLHandle::CURLWriteHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);        // &&RB TODO : Ask Francis.Boily about his server certificate
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    setupCertificateAuthorities(curl);
    setupProxyToCurl(curl);

    auto res = curl_easy_perform(curl);
    if (CURLE_OK != res)
        {
        DATASOURCEACCOUNTCURL_LOG.errorv("downloadBlobSync() : CURL failed with response [%s], while trying to download %ls", curl_easy_strerror(res), url.c_str());
        status = DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
        }
    else if (ISURL(url.c_str()) && !IsResponseOK(response_header))
        {
        status = DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
        }

    if (!response_header.data.empty()) response_header.data.clear();

    curl_handle->free_header_list();

    return status;
    }

bool DataSourceAccountCURL::IsResponseOK(const CURLHandle::CURLDataResponseHeader& response)
    {
    if(response.data.empty())
        {
        DATASOURCEACCOUNTCURL_LOG.error("IsResponseOK() : No data");
        return false;
        }
    if(response.data.count("Content-Length") == 0 && response.data.count("content-length") == 0)
        {
        DATASOURCEACCOUNTCURL_LOG.error("IsResponseOK() : Empty response");
        return false;
        }
    if(response.data.count("HTTP") == 1 && response.data.at("HTTP") != "1.1 200 OK")
        {
        DATASOURCEACCOUNTCURL_LOG.errorv("IsResponseOK() : HTTP error %s", response.data.at("HTTP").c_str());
        return false;
        }
    return true;
    }
void DataSourceAccountCURL::setupProxyToCurl(CURL* curl)
    {
    assert(curl != nullptr);

    if (!proxyServerUrl.empty())
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxyServerUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        if (!proxyUser.empty() && !proxyPassword.empty())
            {
            Utf8String proxyCreds = proxyUser;
            proxyCreds.append(":");
            proxyCreds.append(proxyPassword);
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyCreds.c_str());
            }
        }
    }

void DataSourceAccountCURL::setupCertificateAuthorities(CURL* curl)
    {
    assert(curl != nullptr);

    if (!certificateAuthoritiesUrl.empty())
        {
        curl_easy_setopt(curl, CURLOPT_CAINFO, certificateAuthoritiesUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        }
    }

DataSourceStatus DataSourceAccountCURL::uploadBlobSync(DataSourceURL &url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    struct CURLHandle::CURLDataMemoryBuffer buffer;
    buffer.data.raw_data = source;
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
    //curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_1);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);        // &&RB TODO : Ask Francis.Boily about his server certificate
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,           1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,    CURLHandle::CURLDummyWriteDataCallback); // No output to console
    curl_easy_setopt(curl, CURLOPT_READFUNCTION,     CURLHandle::CURLReadDataCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION,   CURLHandle::CURLWriteHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA,         buffer);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA,       &response_header);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, size);

    setupCertificateAuthorities(curl);
    setupProxyToCurl(curl);


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

DataSourceStatus DataSourceAccountCURL::uploadBlobSync(const DataSourceURL &url, DataSourceBuffer* source)
    {
    struct CURLHandle::CURLDataMemoryBuffer buffer;
    buffer.data.raw_data = source->getBuffer()->data();
    buffer.size = source->getSize();
    struct CURLHandle::CURLDataResponseHeader response_header;

    Utf8String utf8URL = Utf8String(url.c_str());
    std::string contentLength = "Content-Length " + std::to_string(source->getSize());
    std::string contentDisposition = "Content-Disposition: attachment; filename=\"";
    contentDisposition += Utf8String(utf8URL.c_str()).c_str();
    contentDisposition += "\"";

    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();
    CURL* curl = curl_handle->get();

    curl_handle->add_item_to_header("Content-Type: text/plain");
    curl_handle->add_item_to_header(contentLength.c_str());
    curl_handle->add_item_to_header(contentDisposition.c_str());


    curl_easy_setopt(curl, CURLOPT_URL, utf8URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_handle->get_headers());
    //curl_easy_setopt(curl, CURLOPT_HEADEROPT,  CURLHEADER_SEPARATE); // This is for connections with a proxy
    //curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_1);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);        // &&RB TODO : Ask Francis.Boily about his server certificate
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLHandle::CURLDummyWriteDataCallback); // No output to console
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, CURLHandle::CURLReadDataCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CURLHandle::CURLWriteHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, source->getSize());

    setupCertificateAuthorities(curl);
    setupProxyToCurl(curl);


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
            std::map<std::string, std::string>::iterator findIter(header->data.find(line.substr(0, index)));

            if (findIter == header->data.end())
                {
                header->data.insert(std::make_pair(line.substr(0, index), line.substr(index + 1)));
                }
            else //When doing proxy connection multiple http statements are sent, keep the last one.
#if defined(__APPLE__) || ANDROID || defined(__linux__)
            if (strcasecmp(findIter->first.c_str(), "http") == 0)
#else
            if (_stricmp(findIter->first.c_str(), "http") == 0)
#endif
                {
                findIter->second = line.substr(index + 1);
                }
            }
        }

    return size * nmemb;
    }

size_t DataSourceAccountCURL::CURLHandle::CURLWriteDataCallbackRaw(void * contents, size_t size, size_t nmemb, void * userp)
    {
    size_t realsize = size * nmemb;
    struct CURLDataMemoryBuffer *mem = (struct CURLDataMemoryBuffer *)userp;

    memcpy(&mem->data.raw_data[0], (uint8_t*)contents, realsize);
    mem->data.raw_data += realsize;
    mem->size += realsize;

    return realsize;
    }

size_t DataSourceAccountCURL::CURLHandle::CURLWriteDataCallbackVector(void * contents, size_t size, size_t nmemb, void * userp)
    {
    size_t realsize = size * nmemb;
    struct CURLDataMemoryBuffer *mem = (struct CURLDataMemoryBuffer *)userp;

    mem->data.vector->append((uint8_t*)contents, realsize);
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
        memcpy(bufptr, &mem->data.raw_data[0], sizeToRead);
        mem->data.raw_data += sizeToRead;
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
    //curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_1);
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
