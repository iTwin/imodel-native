#include "stdafx.h"
#include "DataSourceAccountCached.h"


DataSourceAccountCached::DataSourceAccountCached(void)
{
    setCacheAccount(nullptr);
}

DataSourceAccountCached::~DataSourceAccountCached(void)
{
                                                            // Note: cache DataSource objects are deleted by the DataSourceManager recursively
}


DataSource *DataSourceAccountCached::getCacheDataSource(SessionName &session)
{
    //NEEDS_WORK_SM : Multiple cahce DataSources needed
    return nullptr;
}

DataSourceStatus DataSourceAccountCached::getFormattedCacheURL(const DataSourceURL & sourceURL, DataSourceURL & cacheURL)
{
                                                            // Construct cache path based on <CacheRootPath>\\<AccountName>\\<SourceURL>
    DataSourceURL    dataPath;
    DataSourceURL    dataPathCollapsed;

    dataPath = getAccountName();
    dataPath.append(getPrefixPath());
    dataPath.append(sourceURL);
    dataPath.collapseDirectories(dataPathCollapsed);

    cacheURL = dataPathCollapsed;

    return DataSourceStatus();
}


DataSourceStatus DataSourceAccountCached::setCaching(DataSourceAccount & cacheAccount, const DataSourceURL & rootPath)
{
    (void) rootPath;

    DataSource *dataSource;
                                                            // Set account used to create cache DataSource
    setCacheAccount(&cacheAccount);
                                                            // Set the root path used to cache this account
//    setCacheRootURL(rootPath);
                                                            // Get a DataSource for caching from the given caching account

                                                            //NEEDS_WORK_SM : Multiple CacheDataSources required
    SessionName nullSession;
    if ((dataSource = cacheAccount.createDataSource(nullSession)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Create_DataSource);
                                                            // Set the cache DataSource
//  setCacheDataSource(dataSource);
                                                            // Return OK
    return DataSourceStatus();
}

void DataSourceAccountCached::setCacheAccount(DataSourceAccount * account)
{
    m_cacheAccount = account;
}

DataSourceAccount * DataSourceAccountCached::getCacheAccount(void)
{
    return m_cacheAccount;
}

unsigned int DataSourceAccountCached::getDefaultNumTransferTasks(void)
    {
    return DATA_SOURCE_SERVICE_DEFAULT_TRANSFER_TASKS;
    }
