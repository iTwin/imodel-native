/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffered.h"
#include "DataSourceCached.h"
#include "DataSourceManager.h"
#include "include/DataSourceAccount.h"

#include <assert.h>
#include <sstream>

DataSourceTransferScheduler::Ptr  DataSourceAccount::getTransferScheduler(void)
    {
    if (dataSourceTransferScheduler == nullptr)
        {
        dataSourceTransferScheduler = new DataSourceTransferScheduler;
        }
    
    return dataSourceTransferScheduler;
    }

unsigned int DataSourceAccount::getDefaultNumTransferTasks(void)
{
                                                            // Default is non scheduled account type (i.e. single connection, no segmentation etc.)
    return 0;
}

void DataSourceAccount::setPrefixPathType(PrefixPathType type)
    {
    prefixPathType = type;
    }


DataSourceAccount::PrefixPathType DataSourceAccount::getPrefixPathType(void) const
    {
    return prefixPathType;
    }

DataSourceAccount::DataSourceAccount(void) : dataSourceManager(nullptr)
    {
    setReferenceCounter(0);
                                                            // Default to using Account level prefix paths. Derived accounts should set this if necessary.
    setPrefixPathType(PrefixPathAccount);
    }

DataSourceAccount::~DataSourceAccount(void)
    {
                                                            // Note: Call destroyAll() before deletion
    assert(getReferenceCounter() == 0);
    }

bool DataSourceAccount::destroyAll(void)
    {
                                                            // Destroy all DataSources associated with this account (and cache DataSources)
        if (destroyDataSources().isFailed())
            return false;

        if (getCacheAccount())
            {
                                                            // Get Cache Account's Service
            DataSourceService *cacheService = getDataSourceManager().getService(getCacheAccount()->getServiceName());
                                                            // Inform Service that Cache Account has been released
            if (cacheService)
                {
                cacheService->releaseAccount(getCacheAccount()->getAccountName());
                CacheWriter::ShutdownCacheWriter();
                }

            }

        return true;
    }

DataSourceStatus DataSourceAccount::destroyDataSources(void)
    {
                                                            // Shut down any potential theaded transfers before destroying DataSources
    dataSourceTransferScheduler = nullptr;
                                                            // Destroy DataSources associated with this Account
    return getDataSourceManager().destroyDataSources(this);
    }

DataSourceStatus DataSourceAccount::destroyDataSource(DataSource * dataSource)
    {
    getDataSourceManager().destroyDataSource(dataSource);
    return DataSourceStatus();
    }

void DataSourceAccount::setDataSourceManager(DataSourceManager &manager)
    {
    dataSourceManager = &manager;
    }

DataSourceManager & DataSourceAccount::getDataSourceManager(void)
    {
    return *dataSourceManager;
    }

DataSourceAccount::DataSourceAccount(const ServiceName & service, const AccountName &account) : DataSourceAccount()
    {
    setServiceName(service);

    setAccountName(account);
                                                            // Default to caching disabled
    setCachingEnabled(false);
    }

DataSourceAccount::DataSourceAccount(const ServiceName & service, const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key) : DataSourceAccount()
    {
    setAccount(service, account, identifier, key);
    }

DataSourceStatus DataSourceAccount::setAccount(const ServiceName &service, const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key)
    {
    if (service.length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    setServiceName(service);

    return DataSourceAccount::setAccount(account, identifier, key);
    }

DataSourceStatus DataSourceAccount::setAccount(const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key)
    {
    if (account.length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    setAccountName(account);

    setAccountIdentifier(identifier);

    setAccountKey(key);

    return DataSourceStatus();
    }

void DataSourceAccount::setServiceName(const ServiceName & name)
    {
    serviceName = name;
    }

const DataSourceAccount::ServiceName & DataSourceAccount::getServiceName(void) const
    {
    return serviceName;
    }


void DataSourceAccount::setAccountName(const AccountName & name)
    {
    accountName = name;
    }

const DataSourceAccount::AccountName & DataSourceAccount::getAccountName(void) const
    {
    return accountName;
    }

void DataSourceAccount::setAccountIdentifier(const AccountIdentifier & identifier)
    {
    accountIdentifier = identifier;
    }

const DataSourceAccount::AccountIdentifier & DataSourceAccount::getAccountIdentifier(void) const
    {
    return accountIdentifier;
    }

void DataSourceAccount::setAccountKey(const AccountKey & key)
    {
    accountKey = key;
    }

const DataSourceAccount::AccountKey DataSourceAccount::getAccountKey(void) const
    {
    return accountKey;
    }

void DataSourceAccount::setAccountSSLCertificatePath(const AccountSSLCertificatePath & path)
    {
    accountSSLCertificatePath = path;
    }

const DataSourceAccount::AccountSSLCertificatePath DataSourceAccount::getAccountSSLCertificatePath(void) const
    {
    return accountSSLCertificatePath;
    }

void DataSourceAccount::setWSGTokenGetterCallback(const std::function<std::string(void)>& )
    {
    // Nothing to do
    }

void DataSourceAccount::setCachingEnabled(bool enabled)
    {
    cachingEnabled = enabled;
    }

bool DataSourceAccount::getCachingEnabled(void)
    {
    return cachingEnabled;
    }

DataSource * DataSourceAccount::createDataSource(const DataSourceName &name, const SessionName &session)
    {
    return getDataSourceManager().createDataSource(name, *this, session);
    }


DataSource * DataSourceAccount::getOrCreateDataSource(const DataSourceName &name, const SessionName &session, bool *created)
    {
    return getDataSourceManager().getOrCreateDataSource(name, *this, session, created);
    }


DataSource * DataSourceAccount::getOrCreateThreadDataSource(const SessionName &session, bool *created)
{
    std::wstringstream      name;
    DataSourceName          dataSourceName;
                                                            // Get thread ID and use as DataSource name
    std::thread::id threadID = std::this_thread::get_id();
    name << threadID;

    dataSourceName = getAccountName() + L"_thread-" + name.str();

    return getDataSourceManager().getOrCreateDataSource(dataSourceName, *this, session, created);
    }


void DataSourceAccount::setPrefixPath(const DataSourceURL & prefix)
    {
    prefixPath = prefix;
    }

const DataSourceURL DataSourceAccount::getPrefixPath(void) const
    {
    return prefixPath;
    }

DataSourceStatus DataSourceAccount::uploadSegments(DataSource &dataSource)
    {

    DataSourceBuffered      *       dataSourceBuffered;
    DataSourceBuffer        *       buffer;
                                                            // Only buffered datasources are supported, so downcast
    if ((dataSourceBuffered = dynamic_cast<DataSourceBuffered *>(&dataSource)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Transfer ownership of the DataSource's buffer
    if ((buffer = dataSourceBuffered->transferBuffer()) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Transfer the buffer to the upload scheduler, where it will eventually be deleted
    getTransferScheduler()->addBuffer(*buffer);
                                                            // Wait for all segments to complete
    //return buffer->waitForSegments(DataSourceBuffered::Timeout(60 * 1000), 10);
    return DataSourceStatus();
    }

DataSourceStatus DataSourceAccount::upload(DataSource &dataSource)
    {

    DataSourceBuffered      *       dataSourceBuffered;
    DataSourceBuffer        *       buffer;
                                                            // Only buffered datasources are supported, so downcast
    if ((dataSourceBuffered = dynamic_cast<DataSourceBuffered *>(&dataSource)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Transfer ownership of the DataSource's buffer
    if ((buffer = dataSourceBuffered->transferBuffer()) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    buffer->setSegmented(false);
                                                            // Transfer the buffer to the upload scheduler, where it will eventually be deleted
    getTransferScheduler()->addBuffer(*buffer);
                                                            // Wait for all segments to complete
    //return buffer->waitForSegments(DataSourceBuffered::Timeout(60 * 1000), 10);
    return DataSourceStatus();
    }

DataSourceStatus DataSourceAccount::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize)
    {
    (void)dataSource;
    (void)dest;
    (void)destSize;
    (void)readSize;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::downloadBlobSync(DataSourceURL &segmentName, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &sessionName)
    {
    (void)segmentName;
    (void)dest;
    (void)readSize;
    (void)size;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::downloadBlobSync(DataSourceURL &url, DataSourceBuffer* buffer, const DataSource::SessionName &session)
    {
    (void)url;
    (void)buffer;
    (void)session;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::uploadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    (void)dataSource;
    (void)source;
    (void)size;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::uploadBlobSync(DataSourceURL & url, const std::wstring & filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    (void)url;
    (void)filename;
    (void)source;
    (void)size;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::uploadBlobSync(const DataSourceURL &segmentName, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    (void)segmentName;
    (void)source;
    (void)size;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::uploadBlobSync(const DataSourceURL &blobPath, DataSourceBuffer *source)
    {
    (void)blobPath;
    (void)source;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::setCaching(DataSourceAccount & cacheAccount, const DataSourceURL & cachingRootPath)
    {
    (void)cacheAccount;
    (void)cachingRootPath;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

void DataSourceAccount::setCacheAccount(DataSourceAccount * account)
    {
    (void)account;

    DataSourceStatus status(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceAccount * DataSourceAccount::getCacheAccount(void)
    {
    DataSourceStatus status(DataSourceStatus::Status_Error_Not_Supported);

    return nullptr;
    }

DataSourceStatus DataSourceAccount::getFormattedCacheURL(const DataSourceURL &sourceURL, DataSourceURL &fullCacheURL)
    {
    (void)sourceURL;
    (void)fullCacheURL;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

DataSourceStatus DataSourceAccount::downloadSegments(DataSource &dataSource, DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize size)
    {
    DataSourceBuffered  *        dataSourceBuffered;
    DataSourceBuffer    *        buffer;

    (void)size;
    (void)dest;
                                                            // Only buffered datasources are supported, so downcast
    if ((dataSourceBuffered = dynamic_cast<DataSourceBuffered *>(&dataSource)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
    //                                                        // Transfer ownership of the DataSource's buffer
    //if ((buffer = dataSourceBuffered->transferBuffer()) == nullptr)
    //    return DataSourceStatus(DataSourceStatus::Status_Error);
    buffer = dataSourceBuffered->getBuffer();
    buffer->setLocator(*dataSourceBuffered);
                                                            // Transfer the buffer to the upload scheduler, where it will eventually be deleted
    getTransferScheduler()->addBuffer(*buffer);
                                                            // Wait for specified timeout
    return buffer->waitForSegments(dataSource.getTimeout());
    }

DataSourceStatus DataSourceAccount::download(DataSource &dataSource, DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize)
    {
    DataSourceBuffered  *        dataSourceBuffered;
    DataSourceBuffer    *        buffer;

    (void)dest;
    (void)readSize;
    (void)destSize;
                                                            // Only buffered datasources are supported, so downcast
    if ((dataSourceBuffered = dynamic_cast<DataSourceBuffered *>(&dataSource)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
    //                                                        // Transfer ownership of the DataSource's buffer
    //if ((buffer = dataSourceBuffered->transferBuffer()) == nullptr)
    //    return DataSourceStatus(DataSourceStatus::Status_Error);
    buffer = dataSourceBuffered->getBuffer();
    buffer->setLocator(*dataSourceBuffered);
                                                            // Transfer the buffer to the upload scheduler
    getTransferScheduler()->addBuffer(*buffer);
                                                            // Wait for specified timeout
    DataSourceStatus status =  buffer->waitForSegments(dataSource.getTimeout());
    readSize = buffer->getReadSize();

    return status;
    }

DataSourceStatus DataSourceAccount::download(DataSource &dataSource, std::vector<DataSourceBuffer::BufferData>& dest)
    {
    DataSourceBuffered  *        dataSourceBuffered;
    DataSourceBuffer    *        buffer;

    (void)dest;

    // Only buffered datasources are supported, so downcast
    if ((dataSourceBuffered = dynamic_cast<DataSourceBuffered *>(&dataSource)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    buffer = dataSourceBuffered->getBuffer();
    buffer->setLocator(*dataSourceBuffered);

    // Transfer the buffer to the upload scheduler
    getTransferScheduler()->addBuffer(*buffer);

    // Wait for specified timeout
    return buffer->waitForSegments(dataSource.getTimeout());
    }