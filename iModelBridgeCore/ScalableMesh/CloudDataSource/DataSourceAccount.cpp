#include "stdafx.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffered.h"
#include "DataSourceManager.h"
#include "include\DataSourceAccount.h"

#include <assert.h>
#include <sstream>

DataSourceTransferScheduler & DataSourceAccount::getTransferScheduler(void)
{
    return transferScheduler;
}

DataSourceAccount::DataSourceAccount(void) : dataSourceManager(nullptr)
{
}

DataSourceAccount::~DataSourceAccount(void)
{
}

void DataSourceAccount::setDataSourceManager(DataSourceManager &manager)
{
    dataSourceManager = &manager;
}

DataSourceManager & DataSourceAccount::getDataSourceManager(void)
{
//    assert(dataSourceManager != nullptr);

    return *dataSourceManager;
}

DataSourceAccount::DataSourceAccount(const ServiceName & service, const AccountName &account)
{
    setServiceName(service);

    setAccountName(account);
}

DataSourceAccount::DataSourceAccount(const ServiceName & service, const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key)
{
    setAccount(service, account, identifier, key);
}

DataSourceStatus DataSourceAccount::setAccount(const ServiceName &service, const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key)
{
    if (service.length() == 0 || account.length() == 0)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    setServiceName(service);

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

DataSource * DataSourceAccount::createDataSource(DataSourceManager::DataSourceName &name)
{
    return getDataSourceManager().createDataSource(name, *this);
}


DataSource * DataSourceAccount::getOrCreateDataSource(DataSourceManager::DataSourceName &name, bool *created)
{
    return getDataSourceManager().getOrCreateDataSource(name, *this, created);
}


DataSource * DataSourceAccount::getOrCreateThreadDataSource(bool *created)
{
    assert(dataSourceManager != nullptr);
    std::wstringstream        name;
    DataSource::Name        dataSourceName;
                                                            // Get thread ID and use as DataSource name
    std::thread::id threadID = std::this_thread::get_id();
    name << threadID;

    dataSourceName = name.str();

    return getDataSourceManager().getOrCreateDataSource(dataSourceName, *this, created);
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

    DataSourceBuffered    *        dataSourceBuffered;
    DataSourceBuffer    *        buffer;
                                                            // Only buffered datasources are supported, so downcast
    if ((dataSourceBuffered = dynamic_cast<DataSourceBuffered *>(&dataSource)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Transfer ownership of the DataSource's buffer
    if((buffer = dataSourceBuffered->transferBuffer()) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Transfer the buffer to the upload scheduler, where it will eventually be deleted
    getTransferScheduler().addBuffer(*buffer);
                                                            // Wait for all segments to complete
    buffer->waitForSegments(DataSourceBuffered::Timeout(1000000));
                                                            // Return OK
    return DataSourceStatus();
}

DataSourceStatus DataSourceAccount::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize)
{
    (void) dataSource;
    (void) dest;
    (void) destSize;
    (void) readSize;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
}

DataSourceStatus DataSourceAccount::downloadBlobSync(const DataSourceURL &segmentName, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size)
{
    (void) segmentName;
    (void) dest;
    (void) readSize;
    (void) size;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
}

DataSourceStatus DataSourceAccount::uploadBlobSync(const DataSourceURL &segmentName, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
{
    (void) segmentName;
    (void) source;
    (void) size;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
}

DataSourceStatus DataSourceAccount::setCaching(DataSourceAccount & cacheAccount, const DataSourceURL & cachingRootPath)
{
    (void) cacheAccount;
    (void) cachingRootPath;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
}

void DataSourceAccount::setCacheAccount(DataSourceAccount * account)
{
    (void) account;

    DataSourceStatus status(DataSourceStatus::Status_Error_Not_Supported);
}

DataSourceAccount * DataSourceAccount::getCacheAccount(void)
{
    DataSourceStatus status(DataSourceStatus::Status_Error_Not_Supported);

    return nullptr;
}

DataSourceStatus DataSourceAccount::getFormattedCacheURL(const DataSourceURL &sourceURL, DataSourceURL &fullCacheURL)
{
    (void) sourceURL;
    (void) fullCacheURL;

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
}

DataSourceStatus DataSourceAccount::downloadSegments(DataSource &dataSource, DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize size)
{
    DataSourceBuffered    *        dataSourceBuffered;
    DataSourceBuffer    *        buffer;

    (void) size;
    (void) dest;
                                                            // Only buffered datasources are supported, so downcast
    if ((dataSourceBuffered = dynamic_cast<DataSourceBuffered *>(&dataSource)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Transfer ownership of the DataSource's buffer
    if((buffer = dataSourceBuffered->transferBuffer()) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);
                                                            // Transfer the buffer to the upload scheduler, where it will eventually be deleted
    getTransferScheduler().addBuffer(*buffer);
                                                            // Wait for specified timeout
    if (buffer->waitForSegments(dataSource.getTimeout()) != ActivitySemaphore::Status::Status_NoTimeout)
    {
                                                            // Return OK
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
    }

    return DataSourceStatus();
}