#pragma once
#include "DataSourceDefs.h"
#include "DataSourceStatus.h"
#include "DataSourceTransferScheduler.h"
#include "DataSource.h"


class DataSource;
class DataSourceManager;

unsigned int const DATA_SOURCE_SERVICE_DEFAULT_TRANSFER_TASKS = 16;


class DataSourceAccount
{

public:

    typedef std::wstring                ServiceName;
    typedef std::wstring                AccountName;
    typedef std::wstring                AccountIdentifier;
    typedef std::wstring                AccountKey;
    typedef std::string                 AccountSSLCertificatePath;

protected:

    DataSourceManager *                 dataSourceManager;
    DataSourceTransferScheduler::Ptr    dataSourceTransferScheduler;

    ServiceName                         serviceName;
    AccountName                         accountName;
    AccountIdentifier                   accountIdentifier;
    AccountKey                          accountKey;
    AccountSSLCertificatePath           accountSSLCertificatePath;
    DataSourceURL                       prefixPath;

protected:

    DataSourceTransferScheduler::Ptr    getTransferScheduler            (void);

    virtual unsigned int                getDefaultNumTransferTasks      (void);

public:
    CLOUD_EXPORT                        DataSourceAccount               (void);
    CLOUD_EXPORT                        DataSourceAccount               (const ServiceName &service, const AccountName &account);
    CLOUD_EXPORT                        DataSourceAccount               (const ServiceName &service, const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

   virtual                             ~DataSourceAccount               (void);

    void                                setDataSourceManager            (DataSourceManager &manager);
    DataSourceManager &                 getDataSourceManager            (void);

    virtual DataSourceStatus            setAccount                      (const ServiceName &service, const AccountName &accountName, const AccountIdentifier &identifier, const AccountKey &key);
            
    void                                setServiceName                  (const ServiceName &name);
    CLOUD_EXPORT    const ServiceName & getServiceName                  (void) const;

    void                                setAccountName                  (const AccountName &name);
    CLOUD_EXPORT    const AccountName & getAccountName                  (void) const;

    void                                setAccountIdentifier            (const AccountIdentifier &identifier);
    const AccountIdentifier &           getAccountIdentifier            (void) const;

    void                                setAccountKey                   (const AccountKey &key);
    const AccountKey                    getAccountKey                   (void) const;

    CLOUD_EXPORT    void                setAccountSSLCertificatePath    (const AccountSSLCertificatePath &path);
    const AccountSSLCertificatePath     getAccountSSLCertificatePath    (void) const;

    
    CLOUD_EXPORT virtual void           setWSGTokenGetterCallback       (const std::function<std::string(void)>& tokenGetter);
    CLOUD_EXPORT virtual void           SetSASTokenGetterCallback       (const std::function<std::string(const Utf8String& docGuid)>& tokenGetter);

    virtual      DataSource       *     createDataSource                (void) = 0;
    CLOUD_EXPORT DataSource       *     createDataSource                (const DataSource::Name &name);

    CLOUD_EXPORT DataSource       *     getOrCreateDataSource           (const DataSource::Name &name, bool *created = nullptr);
    CLOUD_EXPORT DataSource       *     getOrCreateThreadDataSource     (bool *created = nullptr);

            bool                        destroyAll                      (void);

            DataSourceStatus            destroyDataSources              (void);
    virtual DataSourceStatus            destroyDataSource               (DataSource *dataSource);

            DataSourceStatus            uploadSegments                  (DataSource &dataSource);
            DataSourceStatus            downloadSegments                (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize size);

            DataSourceStatus            upload                          (DataSource & dataSource);
            DataSourceStatus            download                        (DataSource & dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize);

    virtual DataSourceStatus            downloadBlobSync                (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
    virtual DataSourceStatus            downloadBlobSync                (DataSourceURL &blobPath, DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
    virtual DataSourceStatus            uploadBlobSync                  (DataSource &dataSource, DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);
    virtual DataSourceStatus            uploadBlobSync                  (DataSourceURL &dataSource, const std::wstring& filename, DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);
    virtual DataSourceStatus            uploadBlobSync                  (const DataSourceURL &blobPath, DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);


    virtual DataSourceStatus            setCaching                      (DataSourceAccount &cacheAccount, const DataSourceURL &cachingRootPath);

    virtual void                        setCacheAccount                 (DataSourceAccount *account);
    virtual DataSourceAccount    *      getCacheAccount                 (void);

    CLOUD_EXPORT virtual void           setPrefixPath                   (const DataSourceURL &url);
    CLOUD_EXPORT const   DataSourceURL  getPrefixPath                   (void) const;

    virtual DataSourceStatus            getFormattedCacheURL            (const DataSourceURL &sourceURL, DataSourceURL &fullCacheURL);

};