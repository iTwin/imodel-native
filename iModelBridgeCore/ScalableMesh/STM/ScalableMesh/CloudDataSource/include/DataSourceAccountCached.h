#pragma once

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

								DataSourceAccountCached		(void);
							   ~DataSourceAccountCached		(void);

		DataSourceStatus		setCaching					(DataSourceAccount &cacheAccount, const DataSourceURL &cachingRootPath);

		void					setCacheAccount				(DataSourceAccount *account);
		DataSourceAccount	*	getCacheAccount				(void);

		DataSourceStatus		getFormattedCacheURL				(const DataSourceURL & sourceURL, DataSourceURL & cacheURL);
};