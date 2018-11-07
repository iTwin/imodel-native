#include "stdafx.h"
#include <assert.h>
#include "DataSourceAccountAzure.h"
#include "DataSourceAzure.h"
#ifdef USE_WASTORAGE
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>
#endif
#include "include/DataSourceAccountAzure.h"
#include <Bentley\BeStringUtilities.h>
#include <Bentley/WString.h>

#ifdef SM_STREAMING_PERF
#include <iostream>
#include <chrono>
std::mutex s_consoleMutex;
#endif

DataSourceAccountAzure::DataSourceAccountAzure(const ServiceName & name, const AccountIdentifier & identifier, const AccountKey & key)
{
    setAccount(name, identifier, key);
                                                            // Default size is set by Service on creation
    setDefaultSegmentSize(0);

                                                            // Multi-threaded segmented transfers used for Azure, so initialize it
    getTransferScheduler()->initializeTransferTasks(getDefaultNumTransferTasks());
}


DataSourceAccountAzure::AzureConnectionString DataSourceAccountAzure::createConnectionString(AccountIdentifier identifier, AccountKey key)
{
    AzureConnectionString    cs;

    cs += std::wstring(L"DefaultEndpointsProtocol=https;");
    cs += std::wstring(L"AccountName=") + identifier + L";";
    cs += L"AccountKey=" + key;

    return cs;
}

DataSourceStatus DataSourceAccountAzure::setConnectionString(const AzureConnectionString string)
{
    if (string.length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);

    connectionString = string;

    return DataSourceStatus();
}

const DataSourceAccountAzure::AzureConnectionString & DataSourceAccountAzure::getConnectionString(void)
{
    return connectionString;
}

#ifdef USE_WASTORAGE
void DataSourceAccountAzure::setStorageAccount(const AzureStorageAccount & account)
{
    storageAccount = account;
}

const DataSourceAccountAzure::AzureStorageAccount & DataSourceAccountAzure::getStorageAccount(void)
{
    return storageAccount;
}

void DataSourceAccountAzure::setBlobClient(const AzureBlobClient &client)
{
    blobClient = client;
}

DataSourceAccountAzure::AzureBlobClient &DataSourceAccountAzure::getBlobClient(void)
{
    return blobClient;
}
#endif

DataSource * DataSourceAccountAzure::createDataSource(const SessionName &session)
{
                                                            // NOTE: This method is for internal use only, don't call this directly.
    DataSourceAzure *   dataSourceAzure;
                                                            // Create a new DataSourceAzure
    dataSourceAzure = new DataSourceAzure(this, session);
    if (dataSourceAzure == nullptr)
        return nullptr;
                                                            // Set the timeout from the account's default (which comes from the Service's default)
    dataSourceAzure->setTimeout(this->getDefaultTimeout());
                                                            // Set the segment size from the account's default (which comes from the Service's default)
    dataSourceAzure->setSegmentSize(this->getDefaultSegmentSize());

    return dataSourceAzure;
}


//DataSourceStatus DataSourceAccountAzure::destroyDataSource(DataSource *dataSource)
//{
//    DataSourceAccount::destroyDataSource(dataSource);
//    if (dataSource)
//    {
//        delete dataSource;
//
//        return DataSourceStatus();
//    }
//
//    return DataSourceStatus(DataSourceStatus::Status_Error);
//}


void DataSourceAccountAzure::setDefaultSegmentSize(DataSourceBuffer::BufferSize size)
{
    defaultSegmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceAccountAzure::getDefaultSegmentSize(void)
{
    return defaultSegmentSize;
}

void DataSourceAccountAzure::setDefaultTimeout(DataSourceBuffer::Timeout time)
    {
    defaultTimeout = time;
    }

DataSourceBuffer::Timeout DataSourceAccountAzure::getDefaultTimeout(void)
    {
    return defaultTimeout;
    }

DataSourceStatus DataSourceAccountAzure::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
{
    if (account.length() == 0 || identifier.length() == 0 /*|| key.length() == 0*/)
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
                                                            // Set details in base class
    DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceAzure"), account, identifier, key);
                                                            // Calculate and store the Azure connection string
    setConnectionString(createConnectionString(identifier, key));

    if (getConnectionString().length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Create storage account reference
#ifdef USE_WASTORAGE
    setStorageAccount(AzureStorageAccount::parse(getConnectionString()));
    if (getStorageAccount().is_initialized() == false)
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Initialize_Subsystem);

    setBlobClient(storageAccount.create_cloud_blob_client());
#endif

    return DataSourceStatus();
}


#ifdef USE_WASTORAGE
DataSourceAccountAzure::AzureContainer DataSourceAccountAzure::initializeContainer(const DataSourceURL & containerName, DataSourceMode mode)
{
    AzureContainer container = getBlobClient().get_container_reference(containerName);

    if (mode == DataSourceMode::DataSourceMode_Write)
    {
        container.create_if_not_exists();
    }

    return container;
}
#endif

#ifdef USE_WASTORAGE
DataSourceStatus DataSourceAccountAzure::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;

    dataSource.getURL(url);

    return downloadBlobSync(url, dest, readSize, destSize, dataSource.getSessionName());
}

DataSourceStatus DataSourceAccountAzure::downloadBlobSync(DataSourceURL &url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session)
{
    DataSourceStatus     status;
    DataSourceURL        containerName;
    DataSourceURL        blobPath;

                                                            // Get first directory as Container and the remainder as the blob's virtual path
    if ((status = url.getContainerAndBlob(containerName, blobPath)).isFailed())
        return status;
                                                            // Make sure container exists
    AzureContainer container = initializeContainer(containerName, DataSourceMode::DataSourceMode_Read);

    azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(blobPath);
    if (blockBlob.is_valid() == false)
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);

    std::streampos p;

    try
    {
        concurrency::streams::producer_consumer_buffer<unsigned char>    pcb;

        concurrency::streams::ostream stream = pcb.create_ostream();

#ifdef SM_STREAMING_PERF
        std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
#endif

        blockBlob.download_to_stream(stream);

#ifdef SM_STREAMING_PERF
        std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
        {
        std::lock_guard<std::mutex> lk(s_consoleMutex);
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << std::endl;
        }
#endif
        p = stream.tell();

        readSize = p;

        stream.close().wait();
        
        pcb.getn(dest, size)
            .then([&readSize,size](size_t nBytes) -> pplx::task<void>
            {
            assert(nBytes == readSize || nBytes == size);
            if (nBytes != size) readSize = nBytes;
            return pplx::task_from_result();
            })
            .wait();
    }
    catch (const azure::storage::storage_exception& e)
    {
        std::wcout << L"Error: " << e.what() << std::endl;
        azure::storage::request_result result = e.result();
        azure::storage::storage_extended_error extended_error = result.extended_error();
        if (!extended_error.message().empty())
            {
            std::wcout << extended_error.message() << std::endl;
            }
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
    }

    return DataSourceStatus();
}

DataSourceStatus DataSourceAccountAzure::uploadBlobSync(DataSource & dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL    url;
    dataSource.getURL(url);

    return uploadBlobSync(url, source, size, dataSource.getSessionName());
    }

DataSourceStatus DataSourceAccountAzure::uploadBlobSync(const DataSourceURL &url, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session)
{
    DataSourceURL        containerName;
    DataSourceURL        blobPath;
    DataSourceStatus    status;
                                                            // Get first directory as Container and the remainder as the blob's virtual path
    if ((status = url.getContainerAndBlob(containerName, blobPath)).isFailed())
        return status;
                                                            // Make sure container exists
    AzureContainer container = initializeContainer(containerName, DataSourceMode::DataSourceMode_Write);

    azure::storage::cloud_block_blob    blockBlob = container.get_block_blob_reference(blobPath);
    if (blockBlob.is_valid() == false)
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);

    try
    {
        concurrency::streams::producer_consumer_buffer<unsigned char>    pcb;

        pcb.putn_nocopy(source, size).wait();

        concurrency::streams::istream stream = pcb.create_istream();

        blockBlob.upload_from_stream(stream, size);

        stream.close().wait();
    }
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
    }


    return DataSourceStatus();
}
#endif

DataSourceAccountAzureCURL::DataSourceAccountAzureCURL(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
    {
                                                            // Paths to data need to use session key as prefix (rather than account)
    setPrefixPathType(PrefixPathSession);

    setAccount(account, identifier, key);
                                                            // Default size is set by Service on creation
    setDefaultSegmentSize(0);

                                                            // Multi-threaded segmented transfers used for Azure, so initialize it
    getTransferScheduler()->initializeTransferTasks(getDefaultNumTransferTasks());

    curl_global_init(CURL_GLOBAL_ALL);

    OpenSSLMutexes::CreateInstance(CRYPTO_num_locks());

    CRYPTO_set_locking_callback(CURLHandle::OpenSSLLockingFunction);
    }

DataSourceStatus DataSourceAccountAzureCURL::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
    {
    if (account.length() == 0 || identifier.length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
                                                                                        // Set details in base class
    DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceAzureCURL"), account, identifier, key);
                                                                                        // Calculate and store the Azure connection string
    //setConnectionString(createConnectionString(identifier, key));
    //
    //if (getConnectionString().length() == 0)
    //    return DataSourceStatus(DataSourceStatus::Status_Error);
    //                                                                                    // Create storage account reference
    //setStorageAccount(AzureStorageAccount::parse(getConnectionString()));
    //if (getStorageAccount().is_initialized() == false)
    //    return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Initialize_Subsystem);
    //
    //setBlobClient(storageAccount.create_cloud_blob_client());

    return DataSourceStatus();
    }


DataSourceStatus DataSourceAccountAzureCURL::downloadBlobSync(DataSourceURL & blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize & readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session)
    {
    auto uriEncodedBlobUrl = BeStringUtilities::UriEncode(Utf8String(blobPath.c_str()).c_str());

    auto azureToken = session.getKeyRemapFunction()();

    if (!azureToken.empty())
        uriEncodedBlobUrl += ("?" + azureToken).c_str();

    DataSourceURL url(L"https://" + this->getAccountIdentifier() + L".blob.core.windows.net/" + DataSourceURL(WString(uriEncodedBlobUrl.c_str(), BentleyCharEncoding::Utf8).c_str()));

    //CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();
    //CURL* curl = curl_handle->get();

    return SuperCURL::downloadBlobSync(url, source, readSize, size, session);
    }

DataSourceStatus DataSourceAccountAzureCURL::downloadBlobSync(DataSourceURL & blobPath, DataSourceBuffer * source, const DataSource::SessionName &session)
    {
    auto uriEncodedBlobUrl = BeStringUtilities::UriEncode(Utf8String(blobPath.c_str()).c_str());

    auto azureToken = session.getKeyRemapFunction()();

    if (!azureToken.empty())
        uriEncodedBlobUrl += ("?" + azureToken).c_str();

    DataSourceURL url(L"https://" + this->getAccountIdentifier() + L".blob.core.windows.net/" + DataSourceURL(WString(uriEncodedBlobUrl.c_str(), BentleyCharEncoding::Utf8).c_str()));

    return SuperCURL::downloadBlobSync(url, source, session);
    }

DataSourceStatus DataSourceAccountAzureCURL::uploadBlobSync(DataSourceURL &blobPath, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL url(L"https://" + this->getAccountIdentifier() + L".blob.core.windows.net/" + blobPath);
    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();
    CURL* curl = curl_handle->get();
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    return SuperCURL::uploadBlobSync(url, filename, source, size);
    }
