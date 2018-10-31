#include "stdafx.h"
#include "DataSourceService.h"
#include "include/DataSourceService.h"

DataSourceService::DataSourceService(DataSourceManager &manager, const ServiceName & name)
{
    setDataSourceManager(manager);

    serviceName = name;
}

DataSourceService::~DataSourceService(void)
{
                                                            // Delete all accounts
    destroyAll();
}


void DataSourceService::setDataSourceManager(DataSourceManager &manager)
{
    dataSourceManager = &manager;
}

void DataSourceService::createAccount(DataSourceManager &manager, DataSourceAccount &account)
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


DataSourceAccount * DataSourceService::getOrCreateAccount(const AccountName & accountName, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey & key)
    {
    std::lock_guard<std::mutex> lock(accountMutex);

    DataSourceAccount *account;

    if ((account = getAccount(accountName)) == nullptr)
        {
        if ((account = createAccount(accountName, identifier, key)) == nullptr)
            return nullptr;
        }
                                                            // Note: Reference counted only on getOrCreate and release
    account->incrementReferenceCounter();

    return account;

    }

DataSourceAccount * DataSourceService::getAccount(const AccountName & accountName)
{
    return get(accountName);
}


bool DataSourceService::releaseAccount(const AccountName & accountName)
    {
    std::lock_guard<std::mutex> lock(accountMutex);

    DataSourceAccount *account = getAccount(accountName);

    if (account)
        {
        if (account->decrementReferenceCounter() == 0)
            {
            return destroyAccount(accountName).isOK();
            }
        }

    return false;
    }

DataSourceStatus DataSourceService::destroyAccount(const AccountName & accountName)
    {
    if (destroy(accountName))
        {
        return DataSourceStatus();
        }

    return DataSourceStatus(DataSourceStatus::Status_Error);
    }
