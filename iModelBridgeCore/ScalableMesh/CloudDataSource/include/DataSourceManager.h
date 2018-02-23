#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "Manager.h"
#include "DataSourceServiceManager.h"


class DataSourceManager : public Manager<DataSource, true>, public DataSourceServiceManager
{
public:

    typedef DataSource::Name                    DataSourceName;
    typedef DataSource::ClientID                ClientID;

protected:

	static DataSourceManager *					dataSourceManager;

protected:
												DataSourceManager		(void);
											   ~DataSourceManager		(void);

public:
	CLOUD_EXPORT static DataSourceManager   *   Get						(void);
	CLOUD_EXPORT static void					Shutdown				(void);

    bool                                        destroyAll              (void);


    DataSource                              *   createDataSource		(const DataSourceName &name, const DataSourceAccount::AccountName &account, DataSource::ClientID clientID, const DataSourceStoreConfig *config = nullptr);
	DataSource                              *   createDataSource		(const DataSourceName &name, DataSourceAccount &account, DataSource::ClientID clientID, const DataSourceStoreConfig *config = nullptr);

	DataSource                              *	getOrCreateDataSource	(const DataSourceName &name, DataSourceAccount &account, DataSource::ClientID clientID, bool *created = nullptr);

	CLOUD_EXPORT DataSourceStatus               destroyDataSource		(DataSource *dataSource);
    DataSourceStatus							destroyDataSources		(DataSourceAccount *dataSourceAccount);
    DataSourceStatus							destroyDataSources      (DataSource::ClientID client);

	DataSourceAccount                       *	initializeAzureTest		(void);

};