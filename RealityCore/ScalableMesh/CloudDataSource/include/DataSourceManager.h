/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "Manager.h"
#include "DataSourceServiceManager.h"


class DataSourceManager : public Manager<DataSource, true>, public DataSourceServiceManager, public DataSourceTypes
{

protected:

	static DataSourceManager *					dataSourceManager;

protected:
												DataSourceManager		        (void);
											   ~DataSourceManager		        (void);

public:
	CLOUD_EXPORT static DataSourceManager   *   Get						        (void);
	CLOUD_EXPORT static void					Shutdown				        (void);

    bool                                        destroyAll                      (void);

    DataSource                              *   createDataSource		        (const DataSourceName &name, const AccountName &account, const SessionName &session, const DataSourceStoreConfig *config = nullptr);
	DataSource                              *   createDataSource		        (const DataSourceName &name, DataSourceAccount &account, const SessionName &session, const DataSourceStoreConfig *config = nullptr);

    CLOUD_EXPORT DataSource                 *	getOrCreateDataSource	        (const DataSourceName &name, DataSourceAccount &account, const SessionName &session, bool *created = nullptr);
    CLOUD_EXPORT DataSource                 *	getOrCreateThreadDataSource     (DataSourceAccount &account, const SessionName &session, bool *created = nullptr);

	CLOUD_EXPORT DataSourceStatus               destroyDataSource		        (DataSource *dataSource);
    CLOUD_EXPORT DataSourceStatus		        destroyDataSources		        (DataSourceAccount *dataSourceAccount);
    CLOUD_EXPORT DataSourceStatus				destroyDataSources              (const SessionName &session);

	DataSourceAccount                       *	initializeAzureTest		        (void);

};