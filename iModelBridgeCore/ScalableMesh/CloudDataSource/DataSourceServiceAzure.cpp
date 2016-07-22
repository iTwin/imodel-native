#include "stdafx.h"
#include "DataSourceServiceAzure.h"
#include "DataSourceAccountAzure.h"
#include "DataSourceAccount.h"
#include "include\DataSourceServiceAzure.h"


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

DataSourceStatus DataSourceServiceAzure::destroyAccount(const AccountName & account)
{
    return Super::destroyAccount(account);
}


