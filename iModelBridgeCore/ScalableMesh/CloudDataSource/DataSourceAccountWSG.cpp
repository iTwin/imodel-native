#include "stdafx.h"
#include <assert.h>
#include <curl/curl.h>
#include <locale>
#include <codecvt>
#include "include/DataSourceAccountWSG.h"
#include "include/DataSourceWSG.h"
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>


DataSourceAccountWSG::DataSourceAccountWSG(const ServiceName & name, const AccountIdentifier & identifier, const AccountKey & key)
{
    setAccount(name, identifier, key);
                                                            // Default size is set by Service on creation
    setDefaultSegmentSize(0);

                                                            // Multi-threaded segmented transfers used for Azure, so initialize it
    getTransferScheduler().initializeTransferTasks(getDefaultNumTransferTasks());
}


unsigned int DataSourceAccountWSG::getDefaultNumTransferTasks(void)
{
    return DATA_SOURCE_SERVICE_WSG_DEFAULT_TRANSFER_TASKS;
}


DataSource * DataSourceAccountWSG::createDataSource(void)
{
                                                            // NOTE: This method is for internal use only, don't call this directly.
    DataSourceWSG *   dataSourceWSG;
                                                            // Create a new DataSourceAzure
    dataSourceWSG = new DataSourceWSG(this);
    if (dataSourceWSG == nullptr)
        return nullptr;
                                                            // Set the timeout from the account's default (which comes from the Service's default)
    dataSourceWSG->setTimeout(this->getDefaultTimeout());
                                                            // Set the segment size from the account's default (which comes from the Service's default)
    dataSourceWSG->setSegmentSize(this->getDefaultSegmentSize());

    return dataSourceWSG;
}


DataSourceStatus DataSourceAccountWSG::destroyDataSource(DataSource *dataSource)
{
    if (dataSource)
    {
        delete dataSource;

        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}


void DataSourceAccountWSG::setDefaultSegmentSize(DataSourceBuffer::BufferSize size)
{
    defaultSegmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceAccountWSG::getDefaultSegmentSize(void)
{
    return defaultSegmentSize;
}

void DataSourceAccountWSG::setDefaultTimeout(DataSourceBuffer::Timeout time)
    {
    defaultTimeout = time;
    }

DataSourceBuffer::Timeout DataSourceAccountWSG::getDefaultTimeout(void)
    {
    return defaultTimeout;
    }

DataSourceStatus DataSourceAccountWSG::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
{
    if (account.length() == 0 )
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
                                                            // Set details in base class
    DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceWSG"), account, identifier, key);

    return DataSourceStatus();
}

void DataSourceAccountWSG::setPrefixPath(const DataSourceURL &prefix)
    {
    DataSourceURL url(this->wsgProtocol + L"//");
    url.append(this->getAccountIdentifier());
    if (!this->wsgPort.empty())
        {
        // using the += operator prevents appending an unwanted separator
        url += (L":" + this->wsgPort);
        }
    url.append(this->wsgVersion);
    url.append(this->wsgAPIID);
    url.append(this->wsgRepository);
    url.append(this->wsgSchema);
    url.append(this->wsgClassName);
    url.append(this->wsgOrganizationID);
    // the organization ID must be followed by a custom separator understood by WSG
    url += (L"~2F" + prefix);

    DataSourceAccount::setPrefixPath(url);
    }


DataSourceStatus DataSourceAccountWSG::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;
    url.overrideDefaultSeparator(L"~2F");

    dataSource.getURL(url);

    // indicate that we want to download the data (instead of just information about the data)
    url += L"/$file";

    return downloadBlobSync(url, dest, readSize, destSize);
}

DataSourceStatus DataSourceAccountWSG::downloadBlobSync(const DataSourceURL &url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size)
{
    try
    {
    CURL *curl_handle;

    struct CURLDataMemoryBuffer buffer;
    buffer.data = dest;
    buffer.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, this->CURLWriteDataCallback);

    std::string utf8URL = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(url);

    std::string utf8Token = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(this->accountKey);
    std::string authToken = "Authorization: Token ";
    authToken.append(utf8Token);

    std::string certificatePath = this->getAccountSSLCertificatePath();

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, authToken.c_str());

    curl_easy_setopt(curl_handle, CURLOPT_URL, utf8URL.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_handle, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0/*1*/);        // &&RB TODO : Ask Francis.Boily about his server certificate
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0/*1*/);
    curl_easy_setopt(curl_handle, CURLOPT_CAINFO, certificatePath.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&buffer);
    
    /* get it! */
    CURLcode res = curl_easy_perform(curl_handle);

    /* check for errors */
    if (CURLE_OK != res)
        {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        throw;
        }
    
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    
    assert(buffer.size <= size);
    readSize = buffer.size;
    }
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
    }

    return DataSourceStatus();
}

DataSourceStatus DataSourceAccountWSG::uploadBlobSync(const DataSourceURL &/*url*/, DataSourceBuffer::BufferData * /*source*/, DataSourceBuffer::BufferSize /*size*/)
{
    try
    {
    // implement curl upload
    throw;
    }
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
    }


    //return DataSourceStatus();
}

size_t DataSourceAccountWSG::CURLWriteDataCallback(void * contents, size_t size, size_t nmemb, void * userp)
    {
    size_t realsize = size * nmemb;
    struct CURLDataMemoryBuffer *mem = (struct CURLDataMemoryBuffer *)userp;
    
    //assert(mem->memory->capacity() >= mem->memory->size() + realsize);
    
    //    mem->memory->assign((Byte*)contents, (Byte*)contents + realsize);
    //mem->memory->insert(mem->memory->end(), (uint8_t*)contents, (uint8_t*)contents + realsize);
    memcpy(mem->data, (uint8_t*)contents, realsize);
    mem->size = realsize;
    
    return realsize;
    }
