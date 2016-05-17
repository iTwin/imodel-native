#include "stdafx.h"
#include "DataSourceServiceFile.h"
#include "DataSourceAccountFile.h"


DataSourceServiceFile::DataSourceServiceFile(DataSourceManager &manager, const ServiceName & service) : DataSourceService(manager, service)
{

}

DataSourceAccount * DataSourceServiceFile::createAccount(const AccountName & account, const DataSourceAccount::AccountIdentifier identifier, const DataSourceAccount::AccountKey & key)
{
	DataSourceAccountFile *	accountFile;

	if ((accountFile = new DataSourceAccountFile(ServiceName(L"DataSourceServiceFile"), account, identifier, key)) == nullptr)
		return accountFile;

	return Manager<DataSourceAccount>::create(account, accountFile);
}

DataSourceStatus DataSourceServiceFile::destroyAccount(const AccountName & account)
{
	(void) account;

	return DataSourceStatus();
}

