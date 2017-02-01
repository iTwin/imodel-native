#include "stdafx.h"
#include <assert.h>
#include "DataSourceAccountAzure.h"
#include "DataSourceAzure.h"
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>
#include "include\DataSourceAccountAzure.h"

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
    getTransferScheduler().initializeTransferTasks(getDefaultNumTransferTasks());
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

DataSource * DataSourceAccountAzure::createDataSource(void)
{
                                                            // NOTE: This method is for internal use only, don't call this directly.
    DataSourceAzure *   dataSourceAzure;
                                                            // Create a new DataSourceAzure
    dataSourceAzure = new DataSourceAzure(this);
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
    if (account.length() == 0 || identifier.length() == 0 || key.length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
                                                            // Set details in base class
    DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceAzure"), account, identifier, key);
                                                            // Calculate and store the Azure connection string
    setConnectionString(createConnectionString(identifier, key));

    if (getConnectionString().length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Create storage account reference
    setStorageAccount(AzureStorageAccount::parse(getConnectionString()));
    if (getStorageAccount().is_initialized() == false)
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Initialize_Subsystem);

    setBlobClient(storageAccount.create_cloud_blob_client());

    return DataSourceStatus();
}


DataSourceAccountAzure::AzureContainer DataSourceAccountAzure::initializeContainer(const DataSourceURL & containerName, DataSourceMode mode)
{
    AzureContainer container = getBlobClient().get_container_reference(containerName);

    if (mode == DataSourceMode::DataSourceMode_Write)
    {
        container.create_if_not_exists();
    }

    return container;
}


DataSourceStatus DataSourceAccountAzure::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;

    dataSource.getURL(url);

    return downloadBlobSync(url, dest, readSize, destSize);
}

DataSourceStatus DataSourceAccountAzure::downloadBlobSync(DataSourceURL &url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size)
{
    DataSourceStatus    status;
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
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
    }

    return DataSourceStatus();
}

DataSourceStatus DataSourceAccountAzure::uploadBlobSync(DataSource & dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL    url;
    dataSource.getURL(url);

    return uploadBlobSync(url, source, size);
    }

DataSourceStatus DataSourceAccountAzure::uploadBlobSync(const DataSourceURL &url, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
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

DataSourceAccountAzureCURL::DataSourceAccountAzureCURL(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
    : SuperCURL(account, identifier, key)
    {
    }

DataSourceStatus DataSourceAccountAzureCURL::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
    {
    if (account.length() == 0 || identifier.length() == 0 || key.length() == 0)
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

DataSourceStatus DataSourceAccountAzureCURL::downloadBlobSync(DataSourceURL & blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize & readSize, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL url(L"https://" + this->getAccountIdentifier() + L".blob.core.windows.net/" + blobPath);

    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();
    CURL* curl = curl_handle->get();
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0/*1*/);  // &&RB TODO : Ask Francis.Boily about his server certificate
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0/*1*/);  // At some point we will have a valid CONNECT certificate and we'll need to reactivate OpenSSL

    return SuperCURL::downloadBlobSync(url, source, readSize, size);
    }

DataSourceStatus DataSourceAccountAzureCURL::uploadBlobSync(DataSourceURL &blobPath, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL url(L"https://" + this->getAccountIdentifier() + L".blob.core.windows.net/" + blobPath);
    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();
    CURL* curl = curl_handle->get();
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    return SuperCURL::uploadBlobSync(url, filename, source, size);
    }
