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

DataSource * DataSourceManager::createDataSource(const DataSourceName & name, const DataSourceAccount::AccountName & accountName, const DataSourceStoreConfig * config)
{
	(void) config;

	DataSource				*	source = nullptr;
	DataSourceAccount		*	account;

	if ((account = DataSourceServiceManager::getAccount(accountName)) == NULL)
		return nullptr;

	if ((source = account->createDataSource()) == nullptr)
		return nullptr;

	if (Manager<DataSource>::create(name, source) == NULL)
		return nullptr;

	return source;
}


DataSource * DataSourceManager::createDataSource(const DataSourceName & name, DataSourceAccount &account, const DataSourceStoreConfig * config)
{
	(void)config;

	DataSource				*	source = nullptr;

	if ((source = account.createDataSource()) == nullptr)
		return nullptr;

	if (Manager<DataSource>::create(name, source) == NULL)
		return nullptr;

	return source;
}


DataSourceStatus DataSourceManager::destroyDataSource(DataSource * dataSource)
{
	if (Manager<DataSource>::destroy(dataSource, true))
	{
		return DataSourceStatus(DataSourceStatus::Status_OK);
	}

	return DataSourceStatus(DataSourceStatus::Status_Error);
}


DataSourceAccount *DataSourceManager::initializeAzureTest(void)
{
	DataSourceAccount::AccountIdentifier		accountIdentifier(L"pcdsustest");
	DataSourceAccount::AccountKey				accountKey(L"3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ==");
	DataSourceStatus							status;
	DataSourceService						*	serviceAzure;
	DataSourceAccount						*	accountAzure;

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