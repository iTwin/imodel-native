#include "stdafx.h"
#include "DataSourceServiceAzure.h"
#include "DataSourceAccountAzure.h"
#include "DataSourceAccount.h"
#include "include\DataSourceServiceAzure.h"


DataSourceServiceAzure::DataSourceServiceAzure(DataSourceManager &manager, const DataSourceService::ServiceName & service) : DataSourceService(manager, service)
{
    setDefaultTimeout(DataSource::Timeout (DATA_SOURCE_SERVICE_AZURE_DEFAULT_TIMEOUT));
    setDefaultSegmentSize(DATA_SOURCE_SERVICE_AZURE_DEFAULT_SEGMENT_SIZE);
}

DataSourceAccount * DataSourceServiceAzure::createAccount(const DataSourceAccount::AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key)
{
    DataSourceAccountAzure *    accountAzure;
                                                            // Create an Azure account with credentials
    if ((accountAzure = new DataSourceAccountAzure(account, identifier, key)) == nullptr)
        return accountAzure;
                                                            // Set up account's default timeout for this type of service
    accountAzure->setDefaultTimeout(getDefaultTimeout());
                                                            // Set up account's default segment size for this type of service
    accountAzure->setDefaultSegmentSize(getDefaultSegmentSize());
                                                            // Inform Service base classes
    DataSourceService::createAccount(getDataSourceManager(), *accountAzure);
                                                            // Add new account to management
    return Manager<DataSourceAccount>::create(account, accountAzure);
}

DataSourceStatus DataSourceServiceAzure::destroyAccount(const AccountName & account)
{
    return Super::destroyAccount(account);
}

void DataSourceServiceAzure::setDefaultSegmentSize(DataSourceBuffer::BufferSize size)
{
    defaultSegmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceServiceAzure::getDefaultSegmentSize(void)
{
    return defaultSegmentSize;
}

void DataSourceServiceAzure::setDefaultTimeout(DataSourceBuffer::Timeout time)
    {
    defaultTimeout = time;
    }

DataSourceBuffer::Timeout DataSourceServiceAzure::getDefaultTimeout(void)
    {
    return defaultTimeout;
    }

