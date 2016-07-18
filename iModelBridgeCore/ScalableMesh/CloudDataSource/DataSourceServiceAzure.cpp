#include "stdafx.h"
#include "DataSourceServiceAzure.h"
#include "DataSourceAccountAzure.h"
#include "DataSourceAccount.h"
#include <assert.h>


DataSourceServiceAzure::DataSourceServiceAzure(DataSourceManager &manager, const DataSourceService::ServiceName & service) : DataSourceService(manager, service)
{

}

DataSourceAccount * DataSourceServiceAzure::createAccount(const DataSourceAccount::AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey &key)
{
    DataSourceAccountAzure *    accountAzure;

    if ((accountAzure = new DataSourceAccountAzure(account, identifier, key)) == nullptr)
        return accountAzure;

    DataSourceService::createAccount(getDataSourceManager(), *accountAzure);

    return Manager<DataSourceAccount>::create(account, accountAzure);
}

DataSourceStatus DataSourceServiceAzure::destroyAccount(const AccountName & accountName)
{
    auto account = Manager<DataSourceAccount>::get(accountName);
    bool isDeleted = Manager<DataSourceAccount>::destroy(account, true);
    assert(isDeleted == true); // asking to destroy an account which does not exist!
    (void) account;

    if (!isDeleted) return DataSourceStatus(DataSourceStatus::Status_Error);

    return DataSourceStatus();
}


