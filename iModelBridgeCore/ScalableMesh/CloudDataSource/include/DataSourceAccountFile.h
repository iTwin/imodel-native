#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceAccount.h"

class DataSourceAccountFile : public DataSourceAccount
{

public:
CLOUD_EXPORT								DataSourceAccountFile		(const ServiceName &service, const AccountName &account);
CLOUD_EXPORT								DataSourceAccountFile		(const ServiceName &service, const AccountName &account, const AccountIdentifier identifier, const AccountKey key);

CLOUD_EXPORT	DataSource				*	createDataSource			(void);
CLOUD_EXPORT	DataSourceStatus			destroyDataSource			(DataSource *dataSource);

};
