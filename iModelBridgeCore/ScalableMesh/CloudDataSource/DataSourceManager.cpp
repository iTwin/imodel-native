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
	(void)						config;

	DataSourceAccount		*	account;

	if ((account = DataSourceServiceManager::getAccount(accountName)) == NULL)
		return nullptr;

	return createDataSource(name, *account, config);
}


DataSource * DataSourceManager::createDataSource(const DataSourceName &name, DataSourceAccount &account, const DataSourceStoreConfig * config)
{
	(void)config;

	DataSource				*	source = nullptr;

	if ((source = account.createDataSource()) == nullptr)
		return nullptr;

	if (Manager<DataSource>::create(name, source) == NULL)
	{
		account.destroyDataSource(source);
		return nullptr;
	}

	return source;
}


/*
			DataSource							*	dataSource;
			DataSource::Name						dataSourceName;
			wstringstream							ss;
			wstringstream							name;

			if (m_dataSourceAccount == nullptr)
				return nullptr;
															// Get DataSourceName based on this thread's ID
			std::thread::id threadID = std::this_thread::get_id();
			name << threadID;
			dataSourceName = name.str();
															// Get the thread's DataSource or create a new one
			dataSource = m_dataSourceAccount->getOrCreateDataSource(dataSourceName);
			if (dataSource == nullptr)
				return nullptr;
															// Enable caching for this DataSource
			dataSource->setCachingEnabled(true);

			destSize = DEFAULT_DATA_SOURCE_BUFFER_SIZE;

			dest.reset(new unsigned char[destSize]);

			return dataSource;
*/

DataSource *DataSourceManager::getOrCreateDataSource(const DataSourceName &name, DataSourceAccount &account, bool *created)
{
	DataSource *	dataSource;
															// Attempt to get the named DataSource
	dataSource = Manager<DataSource>::get(name);
	if (dataSource)
	{
															// If requested, flag that the DataSource existed and was not created
		if (created)
			*created = false;
	}
															// If requested, flag that the DataSource was created
	if (created)
		*created = true;
															// Otherwise, create it
	return createDataSource(name, account);
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