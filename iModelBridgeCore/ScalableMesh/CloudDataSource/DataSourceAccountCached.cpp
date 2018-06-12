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

DataSourceStatus DataSourceAccountCached::getFormattedCacheURL(const DataSourceURL & sourceURL, DataSourceURL & cacheURL)
{
                                                            // Construct cache path based on <CacheRootPath>\\<AccountName>\\<SourceURL>
    DataSourceURL    dataPath;
    DataSourceURL    dataPathCollapsed;
                                                            // <ServiceName>
    //dataPath = getServiceName();
                                                            // <AccountName>
    dataPath = getAccountName();
                                                            // <SourceURL>
    dataPath.append(getPrefixPath());
    dataPath.append(sourceURL);
                                                            // Format path as a single cache file name
    //dataPath.collapseDirectories(dataPathCollapsed);

    cacheURL = dataPath;

    return DataSourceStatus();
}


DataSourceStatus DataSourceAccountCached::setCaching(DataSourceAccount & cacheAccount, const DataSourceURL & rootPath)
{
    (void) rootPath;
                                                            // Set account used to create cache DataSource
    setCacheAccount(&cacheAccount);
                                                            // Set the root path used to cache this account
//  setCacheRootURL(rootPath);
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
    return DATA_SOURCE_ACCOUNT_DEFAULT_TRANSFER_TASKS;
    }
