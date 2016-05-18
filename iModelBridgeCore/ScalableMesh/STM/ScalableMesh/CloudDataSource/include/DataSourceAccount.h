#pragma once

#include "DataSourceStatus.h"
#include "DataSourceTransferScheduler.h"
#include "DataSource.h"


class DataSource;
class DataSourceManager;


class DataSourceAccount
{

public:

	typedef std::wstring			ServiceName;
	typedef std::wstring			AccountName;
	typedef std::wstring			AccountIdentifier;
	typedef std::wstring			AccountKey;

protected:

	DataSourceManager			*	dataSourceManager;

	ServiceName						serviceName;
	AccountName						accountName;
	AccountIdentifier				accountIdentifier;
	AccountKey						accountKey;

	DataSourceTransferScheduler		transferScheduler;

protected:

	DataSourceTransferScheduler	 &	getTransferScheduler		(void);

public:
									DataSourceAccount			(void);
									DataSourceAccount			(const ServiceName &service, const AccountName &account);
									DataSourceAccount			(const ServiceName &service, const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

								   ~DataSourceAccount			(void);

	void							setDataSourceManager		(DataSourceManager &manager);
			DataSourceManager &		getDataSourceManager		(void);

	virtual	DataSourceStatus		setAccount					(const ServiceName &service, const AccountName &accountName, const AccountIdentifier &identifier, const AccountKey &key);
			
			void					setServiceName				(const ServiceName &name);
			const ServiceName &		getServiceName				(void) const;

			void					setAccountName				(const AccountName &name);
			const AccountName &		getAccountName				(void) const;

			void					setAccountIdentifier		(const AccountIdentifier &identifier);
	const	AccountIdentifier &		getAccountIdentifier		(void) const;

			void					setAccountKey				(const AccountKey &key);
	const	AccountKey				getAccountKey				(void) const;

	virtual DataSource			*	createDataSource			(void) = 0;
			DataSource			*	createDataSource			(DataSource::Name &name);

			DataSource			*	getOrCreateDataSource		(DataSource::Name &name, bool *created = nullptr);


	virtual DataSourceStatus		destroyDataSource			(DataSource *dataSource) = 0;

	DataSourceStatus				uploadSegments				(DataSource &dataSource);
	DataSourceStatus				downloadSegments			(DataSource & dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize size);

	virtual DataSourceStatus		downloadBlobSync			(const DataSourceURL &blobPath, DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);
	virtual DataSourceStatus		uploadBlobSync				(const DataSourceURL &blobPath, DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);


	virtual DataSourceStatus		setCaching					(DataSourceAccount &cacheAccount, const DataSourceURL &cachingRootPath);

	virtual	void					setCacheAccount				(DataSourceAccount *account);
	virtual DataSourceAccount	*	getCacheAccount				(void);

	virtual void					setCacheRootURL				(const DataSourceURL &root);
	virtual const DataSourceURL	*	getCacheRootURL				(void);

	virtual DataSourceStatus		getFullCacheURL				(const DataSourceURL &sourceURL, DataSourceURL &fullCacheURL);

};