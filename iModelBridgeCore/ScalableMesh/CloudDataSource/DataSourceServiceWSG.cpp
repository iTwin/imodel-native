#include "stdafx.h"
#include "DataSourceServiceWSG.h"
#include "DataSourceAccountWSG.h"
#include "DataSourceAccount.h"
#include "include\DataSourceServiceWSG.h"


DataSourceServiceWSG::DataSourceServiceWSG(DataSourceManager &manager, const DataSourceService::ServiceName & service) : DataSourceService(manager, service)
{
    setDefaultTimeout(DataSource::Timeout (DATA_SOURCE_SERVICE_WSG_DEFAULT_TIMEOUT));
    setDefaultSegmentSize(DATA_SOURCE_SERVICE_WSG_DEFAULT_SEGMENT_SIZE);
}

DataSourceAccount * DataSourceServiceWSG::createAccount(const DataSourceAccount::AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key)
{
    DataSourceAccountWSG *    accountWSG;
                                                            // Create an WSG account with credentials
    if ((accountWSG = new DataSourceAccountWSG(account, identifier, key)) == nullptr)
        return accountWSG;
                                                            // Set up account's default timeout for this type of service
    accountWSG->setDefaultTimeout(getDefaultTimeout());
                                                            // Set up account's default segment size for this type of service
    accountWSG->setDefaultSegmentSize(getDefaultSegmentSize());
                                                            // Inform Service base classes
    DataSourceService::createAccount(getDataSourceManager(), *accountWSG);
                                                            // Add new account to management
    return Manager<DataSourceAccount>::create(account, accountWSG);
}

DataSourceStatus DataSourceServiceWSG::destroyAccount(const AccountName & account)
{
    return Super::destroyAccount(account);
}

void DataSourceServiceWSG::setDefaultSegmentSize(DataSourceBuffer::BufferSize size)
{
    defaultSegmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceServiceWSG::getDefaultSegmentSize(void)
{
    return defaultSegmentSize;
}

void DataSourceServiceWSG::setDefaultTimeout(DataSourceBuffer::Timeout time)
    {
    defaultTimeout = time;
    }

DataSourceBuffer::Timeout DataSourceServiceWSG::getDefaultTimeout(void)
    {
    return defaultTimeout;
    }

