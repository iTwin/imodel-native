#include "stdafx.h"
#include "DataSourceService.h"
#include "include\DataSourceService.h"

DataSourceService::DataSourceService(DataSourceManager &manager, const ServiceName & name)
{
	setDataSourceManager(manager);

	serviceName = name;
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
//	assert(dataSourceManger != nullptr);

	return *dataSourceManager;
}

const DataSourceService::ServiceName & DataSourceService::getServiceName(void)
{
	return serviceName;
}

DataSourceAccount * DataSourceService::getAccount(const AccountName & accountName)
{
	DataSourceAccount *	account;

	for (auto i : items)
	{
		account = i.second;
		if (account)
		{
			if (account->getAccountName() == accountName)
			{
				return account;
			}
		}
	}

	return nullptr;
}
