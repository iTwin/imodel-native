#include "stdafx.h"
#include "DataSourceManager.h"
#include "DataSourceFile.h"
#include "DataSourceService.h"
#include "DataSourceServiceManager.h"
#include "DataSourceAzure.h"
#include "DataSourceServiceFile.h"
#include "DataSourceServiceAzure.h"
#include "include\DataSourceManager.h"



DataSourceManager::DataSourceManager(void) : DataSourceServiceManager(*this)
{

}

DataSourceManager::~DataSourceManager(void)
{
    
}

DataSource * DataSourceManager::createDataSource(const DataSourceName & name, const DataSourceAccount::AccountName & accountName, const DataSourceStoreConfig * config)
{
    (void)                   config;

    DataSourceAccount   *    account;

    if ((account = DataSourceServiceManager::getAccount(accountName)) == NULL)
        return nullptr;

    return createDataSource(name, *account, config);
}


DataSource * DataSourceManager::createDataSource(const DataSourceName &name, DataSourceAccount &account, const DataSourceStoreConfig * config)
{
    (void)config;

    DataSource                *    source = nullptr;

    if ((source = account.createDataSource()) == nullptr)
        return nullptr;

    if (Manager<DataSource>::create(name, source) == NULL)
    {
        account.destroyDataSource(source);
        return nullptr;
    }

    return source;
}

DataSource *DataSourceManager::getOrCreateDataSource(const DataSourceName &name, DataSourceAccount &account, bool *created)
{
    DataSource *    dataSource;
                                                            // Attempt to get the named DataSource
    dataSource = Manager<DataSource>::get(name);
    if (dataSource)
    {
                                                            // If requested, flag that the DataSource existed and was not created
        if (created)
            *created = false;
                                                            // Return the found DataSource
        return dataSource;
    }
                                                            // If requested, flag that the DataSource was created
    if (created)
        *created = true;
                                                            // Otherwise, create it
    return createDataSource(name, account);
}


DataSourceStatus DataSourceManager::destroyDataSource(DataSource * dataSource)
{
    if (dataSource == nullptr)
        return DataSourceStatus();

    DataSourceStatus    status;
                                                            // If dataSource has a cache DataSource, destroy it
                                                            // Note: This is recursive because cache datasources may have their own cache data sources
    status = destroyDataSource(dataSource->getCacheDataSource());
    if (status.isFailed())
    {
        return status;
    }
                                                            // Then destroy the main data source itself
    if (Manager<DataSource>::destroy(dataSource, true))
    {
        return DataSourceStatus(DataSourceStatus::Status_OK);
    }

    return DataSourceStatus(DataSourceStatus::Status_OK);
}


DataSourceStatus DataSourceManager::destroyDataSources(DataSourceAccount * dataSourceAccount)
{
    if (dataSourceAccount == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    bool deleted;

    Manager<DataSource>::ApplyFunction deleteFirstAccountDataSource = [this, dataSourceAccount, &deleted]( Manager<DataSource>::Iterator it) -> bool
    {
        if (it->second)
        {
                                                            // If DataSource belongs to DataSourceAccount
            if (it->second->getAccount() == dataSourceAccount)
            {
                                                            // Destroy the DataSource
                destroyDataSource(it->second);
                                                            // Return deleted
                deleted = true;
                                                            // Return don't traverse any more
                return false;
            }
        }
                                                            // Not found, so return not deleted
        deleted = false;
                                                            // Return continue traversing
        return true;
    };
                                                            // Iteratively delete the first found DataSource belonging to the given DataSourceAccount
                                                            // This is done this way for container safety, because due to the recursive nature of
                                                            // Deleting DataSource objects, multiple deletions may occur
    do
    {
        deleted = false;
                                                            // Delete account's DataSources
        Manager<DataSource>::apply(deleteFirstAccountDataSource);

    } while (deleted);


    return DataSourceStatus();
}


DataSourceAccount *DataSourceManager::initializeAzureTest(void)
{
    DataSourceAccount::AccountIdentifier        accountIdentifier(L"pcdsustest");
    DataSourceAccount::AccountKey               accountKey(L"3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ==");
    DataSourceStatus                            status;
    DataSourceService                        *  serviceAzure;
    DataSourceAccount                        *  accountAzure;

                                                            // Get the Azure service
    serviceAzure = getService(DataSourceService::ServiceName(L"DataSourceServiceAzure"));
    if(serviceAzure == nullptr)
        return nullptr;
                                                            // Create an account on Azure
    accountAzure = serviceAzure->createAccount(DataSourceAccount::AccountName(L"AzureAccount"), accountIdentifier, accountKey);
    if(accountAzure == nullptr)
        return nullptr;

    return accountAzure;
}