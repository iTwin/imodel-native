#include "stdafx.h"
#include "DataSourceManager.h"
#include "DataSourceFile.h"
#include "DataSourceService.h"
#include "DataSourceServiceManager.h"
#include "DataSourceAzure.h"
#include "DataSourceServiceFile.h"
#ifdef USE_WASTORAGE
#include "DataSourceServiceAzure.h"
#endif
#include "include/DataSourceManager.h"
#include <assert.h>

DataSourceManager *DataSourceManager::dataSourceManager = nullptr;


DataSourceManager *DataSourceManager::Get(void)
{
	if (dataSourceManager == nullptr)
	{
		dataSourceManager = new DataSourceManager;
	}

	return dataSourceManager;
}


void DataSourceManager::Shutdown(void)
{
	if (dataSourceManager != nullptr)
	{
		dataSourceManager->destroyAll();

		delete dataSourceManager;

		dataSourceManager = nullptr;
	}
}

bool DataSourceManager::destroyAll(void)
{
    bool    result;
                                                            // Destroy all services, their accounts and the account DataSources
    result = DataSourceServiceManager::destroyAll();

    return result;
}


DataSourceManager::DataSourceManager(void) : DataSourceServiceManager(*this)
{

}

DataSourceManager::~DataSourceManager(void)
{
    
}

DataSource * DataSourceManager::createDataSource(const DataSourceName & name, const DataSourceAccount::AccountName & accountName, const SessionName &session, const DataSourceStoreConfig * config)
{
    (void)                   config;

    DataSourceAccount   *    account;

    if ((account = DataSourceServiceManager::getAccount(accountName)) == NULL)
        return nullptr;

    return createDataSource(name, *account, session, config);
}


DataSource * DataSourceManager::createDataSource(const DataSourceName &name, DataSourceAccount &account, const SessionName &session, const DataSourceStoreConfig * config)
{
    (void)config;

    DataSource *source = nullptr;

    if ((source = account.createDataSource(session)) == nullptr)
        return nullptr;

    source->setName(name);
                                                            // Inherit cache enabled state from Account's setting
    source->setCachingEnabled(account.getCachingEnabled());

    if (Manager<DataSource, true>::create(name, source) == NULL)
    {
        account.destroyDataSource(source);
        return nullptr;
    }

    return source;
}

DataSource *DataSourceManager::getOrCreateDataSource(const DataSourceName &name, DataSourceAccount &account, const SessionName &session, bool *created)
{
    DataSource *    dataSource;
                                                            // Attempt to get the named DataSource
    dataSource = Manager<DataSource, true>::get(name);
    if (dataSource)
    {
                                                            // If requested, flag that the DataSource existed and was not created
        if (created)
            *created = false;
                                                            // Set up DataSource for use with the given Session
        dataSource->setSessionName(session);

                                                            // Make sure the dataSource has the correct prefix path
        dataSource->setPrefixPath(account.getPrefixPath());

                                                            // Return the found DataSource
        //assert(!dataSource->isValid() || dataSource->isEmpty());
        return dataSource;
    }
                                                            // If requested, flag that the DataSource was created
    if (created)
        *created = true;
                                                            // Otherwise, create it
    return createDataSource(name, account, session);
}


DataSource * DataSourceManager::getOrCreateThreadDataSource(DataSourceAccount &account, const SessionName &session, bool * created)
    {
    return account.getOrCreateThreadDataSource(session, created);
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
    if (Manager<DataSource, true>::destroy(dataSource))
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

    Manager<DataSource, false>::ApplyFunction deleteFirstAccountDataSource = [this, dataSourceAccount, &deleted](Manager<DataSource, false>::Iterator it) -> bool
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
        Manager<DataSource, true>::apply(deleteFirstAccountDataSource);

    } while (deleted);


    return DataSourceStatus();
}


DataSourceStatus DataSourceManager::destroyDataSources(const SessionName &session)
{
    bool deleted;

    Manager<DataSource, false>::ApplyFunction deleteFirstClientDataSource = [this, session, &deleted](Manager<DataSource, false>::Iterator it) -> bool
    {
        if (it->second)
        {
                                                            // If DataSource belongs to Client
            if (it->second->getSessionName() == session)
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
                                                            // Iteratively delete the first found DataSource belonging to the given Client
                                                            // IMPORTANT;
                                                            // This is done this way for container safety, because due to the recursive nature of
                                                            // Deleting DataSource objects, multiple deletions may occur
    do
    {
        deleted = false;
                                                            // Delete account's DataSources
        Manager<DataSource, true>::apply(deleteFirstClientDataSource);

    } while (deleted);


    return DataSourceStatus();
}



#ifdef USE_WASTORAGE
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
#endif