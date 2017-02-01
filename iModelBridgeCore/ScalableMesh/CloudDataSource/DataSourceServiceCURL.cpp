#include "stdafx.h"
#include "DataSourceServiceCURL.h"
#include "DataSourceAccountCURL.h"
#include "DataSourceAccount.h"
#include "include\DataSourceServiceCURL.h"


DataSourceServiceCURL::DataSourceServiceCURL(DataSourceManager &manager, const DataSourceService::ServiceName & service) : DataSourceService(manager, service)
{
    setDefaultTimeout(DataSource::Timeout (DATA_SOURCE_SERVICE_CURL_DEFAULT_TIMEOUT));
    setDefaultSegmentSize(DATA_SOURCE_SERVICE_CURL_DEFAULT_SEGMENT_SIZE);
}

DataSourceAccount * DataSourceServiceCURL::createAccount(const DataSourceAccount::AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key)
{
    DataSourceAccountCURL *    accountCURL;
                                                            // Create an WSG account with credentials
    if ((accountCURL = new DataSourceAccountCURL(account, identifier, key)) == nullptr)
        return accountCURL;
                                                            // Set up account's default timeout for this type of service
    accountCURL->setDefaultTimeout(getDefaultTimeout());
                                                            // Set up account's default segment size for this type of service
    accountCURL->setDefaultSegmentSize(getDefaultSegmentSize());
                                                            // Inform Service base classes
    DataSourceService::createAccount(getDataSourceManager(), *accountCURL);
                                                            // Add new account to management
    return Manager<DataSourceAccount,true>::create(account, accountCURL);
}

DataSourceStatus DataSourceServiceCURL::destroyAccount(const AccountName & account)
{
    return Super::destroyAccount(account);
}

void DataSourceServiceCURL::setDefaultSegmentSize(DataSourceBuffer::BufferSize size)
{
    defaultSegmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceServiceCURL::getDefaultSegmentSize(void)
{
    return defaultSegmentSize;
}

void DataSourceServiceCURL::setDefaultTimeout(DataSourceBuffer::Timeout time)
    {
    defaultTimeout = time;
    }

DataSourceBuffer::Timeout DataSourceServiceCURL::getDefaultTimeout(void)
    {
    return defaultTimeout;
    }

