#pragma once

#include "DataSource.h"
#include "Manager.h"
#include "DataSourceServiceManager.h"

class DataSourceManager : public Manager<DataSource>, public DataSourceServiceManager
{
public:

	typedef Manager<DataSource>::ItemName				DataSourceName;

protected:

public:
								DataSourceManager		(void);

	DataSource			*		createDataSource		(const DataSourceName &name, const DataSourceAccount::AccountName &account, const DataSourceStoreConfig *config = nullptr);
	DataSource			*		createDataSource		(const DataSourceName &name, DataSourceAccount &account, const DataSourceStoreConfig *config = nullptr);
	DataSourceStatus			destroyDataSource		(DataSource *dataSource);

	DataSourceAccount	*		initializeAzureTest		(void);

};