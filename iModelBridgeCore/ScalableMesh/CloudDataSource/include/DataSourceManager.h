#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "Manager.h"
#include "DataSourceServiceManager.h"

class DataSourceManager : public Manager<DataSource>, public DataSourceServiceManager
{
public:

	typedef DataSource::Name	DataSourceName;

protected:

public:
CLOUD_EXPORT								DataSourceManager		(void);

CLOUD_EXPORT	DataSource			*		createDataSource		(const DataSourceName &name, const DataSourceAccount::AccountName &account, const DataSourceStoreConfig *config = nullptr);
CLOUD_EXPORT	DataSource			*		createDataSource		(const DataSourceName &name, DataSourceAccount &account, const DataSourceStoreConfig *config = nullptr);

CLOUD_EXPORT	DataSource			*		getOrCreateDataSource	(const DataSourceName &name, DataSourceAccount &account, bool *created = nullptr);

CLOUD_EXPORT	DataSourceStatus			destroyDataSource		(DataSource *dataSource);

CLOUD_EXPORT	DataSourceAccount	*		initializeAzureTest		(void);

};