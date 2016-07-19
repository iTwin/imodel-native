#pragma once
#include "DataSourceDefs.h"
#include "DataSourceStatus.h"
#include "DataSourceTransferScheduler.h"
#include "DataSource.h"


class DataSource;
class DataSourceManager;


class DataSourceAccount
{

public:

    typedef std::wstring                ServiceName;
    typedef std::wstring                AccountName;
    typedef std::wstring                AccountIdentifier;
    typedef std::wstring                AccountKey;

protected:

    DataSourceManager *                 dataSourceManager;

    ServiceName                         serviceName;
    AccountName                         accountName;
    AccountIdentifier                   accountIdentifier;
    AccountKey                          accountKey;
    DataSourceURL                       prefixPath;

    DataSourceTransferScheduler         transferScheduler;

protected:

    DataSourceTransferScheduler &       getTransferScheduler            (void);

    virtual unsigned int                getDefaultNumTransferTasks      (void);

public:
CLOUD_EXPORT                            DataSourceAccount               (void);
CLOUD_EXPORT                            DataSourceAccount               (const ServiceName &service, const AccountName &account);
CLOUD_EXPORT                            DataSourceAccount               (const ServiceName &service, const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

   virtual                             ~DataSourceAccount               (void);

    void                                setDataSourceManager            (DataSourceManager &manager);
    DataSourceManager &                 getDataSourceManager            (void);

    virtual DataSourceStatus            setAccount                      (const ServiceName &service, const AccountName &accountName, const AccountIdentifier &identifier, const AccountKey &key);
            
    void                                setServiceName                  (const ServiceName &name);
    const ServiceName &                 getServiceName                  (void) const;

    void                                setAccountName                  (const AccountName &name);
    const AccountName &                 getAccountName                  (void) const;

    void                                setAccountIdentifier            (const AccountIdentifier &identifier);
    const AccountIdentifier &           getAccountIdentifier            (void) const;

    void                                setAccountKey                   (const AccountKey &key);
    const AccountKey                    getAccountKey                   (void) const;

    virtual DataSource            *     createDataSource                (void) = 0;
            DataSource            *     createDataSource                (DataSource::Name &name);

            DataSource            *     getOrCreateDataSource           (DataSource::Name &name, bool *created = nullptr);
    CLOUD_EXPORT DataSource       *     getOrCreateThreadDataSource     (bool *created = nullptr);

            DataSourceStatus            destroyDataSources              (void);
    virtual DataSourceStatus            destroyDataSource               (DataSource *dataSource) = 0;

            DataSourceStatus            uploadSegments                  (DataSource &dataSource);
            DataSourceStatus            downloadSegments                (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize size);

    virtual DataSourceStatus            downloadBlobSync                (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
    virtual DataSourceStatus            downloadBlobSync                (const DataSourceURL &blobPath, DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
    virtual DataSourceStatus            uploadBlobSync                  (const DataSourceURL &blobPath, DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);


    virtual DataSourceStatus            setCaching                      (DataSourceAccount &cacheAccount, const DataSourceURL &cachingRootPath);

    virtual void                        setCacheAccount                 (DataSourceAccount *account);
    virtual DataSourceAccount    *      getCacheAccount                 (void);

    CLOUD_EXPORT void                   setPrefixPath                   (const DataSourceURL &url);
    const   DataSourceURL               getPrefixPath                   (void) const;

    virtual DataSourceStatus            getFormattedCacheURL            (const DataSourceURL &sourceURL, DataSourceURL &fullCacheURL);

};