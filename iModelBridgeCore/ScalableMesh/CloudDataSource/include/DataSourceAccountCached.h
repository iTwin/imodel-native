#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceURL.h"


class DataSourceAccountCached : public DataSourceAccount
{
protected:

	DataSourceAccount		*	cacheAccount;
	DataSourceURL				cacheRoot;
	DataSource				*	cacheDataSource;

protected:

		void					setCacheDataSource			(DataSource *dataSource);
		DataSource			*	getCacheDataSource			(void);

public:

CLOUD_EXPORT								DataSourceAccountCached		(void);
CLOUD_EXPORT							   ~DataSourceAccountCached		(void);

CLOUD_EXPORT		DataSourceStatus		setCaching					(DataSourceAccount &cacheAccount, const DataSourceURL &cachingRootPath);

CLOUD_EXPORT		void					setCacheAccount				(DataSourceAccount *account);
CLOUD_EXPORT		DataSourceAccount	*	getCacheAccount				(void);

CLOUD_EXPORT		DataSourceStatus		getFormattedCacheURL				(const DataSourceURL & sourceURL, DataSourceURL & cacheURL);
};