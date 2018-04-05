#pragma once
#include "DataSourceDefs.h"
#include <was/storage_account.h>
#include <was/blob.h>
#include <string>

#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceAccountCached.h"
#include "DataSourceAccountCURL.h"
#include "DataSourceBuffer.h"
#include "DataSourceMode.h"

#ifdef SM_STREAMING_PERF
#include <mutex>
extern std::mutex s_consoleMutex;
#endif


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
    DataSourceBuffer::BufferSize            defaultSegmentSize;
    DataSourceBuffer::Timeout               defaultTimeout;

protected:

    AzureConnectionString                   createConnectionString              (AccountIdentifier identifier, AccountKey key);

    DataSourceStatus                        setConnectionString                 (const AzureConnectionString string);
    const AzureConnectionString        &    getConnectionString                 (void);

    void                                    setStorageAccount                   (const AzureStorageAccount &account);
    const AzureStorageAccount        &      getStorageAccount                   (void);

    void                                    setBlobClient                       (const AzureBlobClient &client);
    AzureBlobClient                    &    getBlobClient                       (void);


public:
                                            DataSourceAccountAzure              (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountAzure              (void) = default;

        void                                setDefaultSegmentSize               (DataSourceBuffer::BufferSize size);
        DataSourceBuffer::BufferSize        getDefaultSegmentSize               (void);

        void                                setDefaultTimeout                   (DataSourceBuffer::Timeout time);
        DataSourceBuffer::Timeout           getDefaultTimeout                   (void);

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

        DataSource                   *      createDataSource                    (const SessionName &session);
      //DataSourceStatus                    destroyDataSource                   (DataSource *dataSource);

        AzureContainer                      initializeContainer                 (const DataSourceURL &containerName, DataSourceMode mode);

        DataSourceStatus                    downloadBlobSync                    (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
        DataSourceStatus                    downloadBlobSync                    (DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session);
        DataSourceStatus                    uploadBlobSync                      (DataSource & dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session);
};

class DataSourceAccountAzureCURL : public DataSourceAccountCURL
    {
    protected:

        typedef       DataSourceAccountAzure    SuperAzure;
        typedef       DataSourceAccountCURL     SuperCURL;

    public:
                                            DataSourceAccountAzureCURL          (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountAzureCURL          (void) = default;

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

        DataSourceStatus                    downloadBlobSync                    (DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session);
        DataSourceStatus                    uploadBlobSync                      (DataSourceURL &blobPath, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);

    };
