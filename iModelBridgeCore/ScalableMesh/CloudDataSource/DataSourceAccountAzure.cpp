#include "stdafx.h"
#include <assert.h>
#include "DataSourceAccountAzure.h"
#include "DataSourceAzure.h"
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>
#include "include\DataSourceAccountAzure.h"


DataSourceAccountAzure::DataSourceAccountAzure(const ServiceName & name, const AccountIdentifier & identifier, const AccountKey & key)
{
    setAccount(name, identifier, key);
                                                            // Default size is set by Service on creation
    setDefaultSegmentSize(0);

                                                            // Multi-threaded segmented transfers used for Azure, so initialize it
    getTransferScheduler().initializeTransferTasks(getDefaultNumTransferTasks());
}


unsigned int DataSourceAccountAzure::getDefaultNumTransferTasks(void)
{
    return DATA_SOURCE_SERVICE_AZURE_DEFAULT_TRANSFER_TASKS;
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


DataSourceStatus DataSourceAccountAzure::destroyDataSource(DataSource *dataSource)
{
    if (dataSource)
    {
        delete dataSource;

        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}


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

        blockBlob.download_to_stream(stream);

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
