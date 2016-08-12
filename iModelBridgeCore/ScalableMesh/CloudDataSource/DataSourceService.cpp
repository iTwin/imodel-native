#include "stdafx.h"
#include "DataSourceService.h"
#include "include\DataSourceService.h"

DataSourceService::DataSourceService(DataSourceManager &manager, const ServiceName & name)
{
    setDataSourceManager(manager);

    serviceName = name;
}

DataSourceService::~DataSourceService(void)
{
                                                            // Delete all accounts
    destroyAll(true);
}


void DataSourceService::setDataSourceManager(DataSourceManager &manager)
{
    dataSourceManager = &manager;
}

void DataSourceService::createAccount(DataSourceManager & manager, DataSourceAccount &account)
{
    account.setDataSourceManager(manager);
}

DataSourceManager &DataSourceService::getDataSourceManager(void)
{
    return *dataSourceManager;
}

const DataSourceService::ServiceName & DataSourceService::getServiceName(void)
{
    return serviceName;
}

DataSourceStatus DataSourceService::destroyAccount(const AccountName & accountName)
{
    if (destroy(accountName, true))
    {
        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}

DataSourceAccount * DataSourceService::getAccount(const AccountName & accountName)
{
    return get(accountName);
}
