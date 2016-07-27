#pragma once
#include "DataSourceDefs.h"
#include <was/storage_account.h>
#include <was/blob.h>
#include <string>

#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceAccountCached.h"
#include "DataSourceBuffer.h"
#include "DataSourceMode.h"

unsigned int const DATA_SOURCE_SERVICE_AZURE_DEFAULT_TRANSFER_TASKS = 16;


class DataSourceAccountAzure : public DataSourceAccountCached
{


protected:

    typedef std::wstring                                AzureConnectionString;
    typedef azure::storage::cloud_storage_account       AzureStorageAccount;
    typedef azure::storage::cloud_blob_client           AzureBlobClient;
    typedef azure::storage::cloud_blob_container        AzureContainer;

protected:

    AzureStorageAccount                     storageAccount;
    AzureBlobClient                         blobClient;
    AzureConnectionString                   connectionString;

protected:

    AzureConnectionString                   createConnectionString              (AccountIdentifier identifier, AccountKey key);

    DataSourceStatus                        setConnectionString                 (const AzureConnectionString string);
    const AzureConnectionString        &    getConnectionString                 (void);

    void                                    setStorageAccount                   (const AzureStorageAccount &account);
    const AzureStorageAccount        &      getStorageAccount                   (void);

    void                                    setBlobClient                       (const AzureBlobClient &client);
    AzureBlobClient                    &    getBlobClient                       (void);

    unsigned int                            getDefaultNumTransferTasks          (void);


public:
                                            DataSourceAccountAzure              (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountAzure              (void) = default;

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

        DataSource                   *      createDataSource                    (void);
        DataSourceStatus                    destroyDataSource                   (DataSource *dataSource);

        AzureContainer                      initializeContainer                 (const DataSourceURL &containerName, DataSourceMode mode);

        DataSourceStatus                    downloadBlobSync                    (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
        DataSourceStatus                    downloadBlobSync                    (const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
};
