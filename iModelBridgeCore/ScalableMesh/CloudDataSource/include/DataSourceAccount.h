#pragma once
#include "DataSourceDefs.h"
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
	DataSourceURL					prefixPath;

	DataSourceTransferScheduler		transferScheduler;

protected:

	DataSourceTransferScheduler	 &	getTransferScheduler		(void);

public:
CLOUD_EXPORT									DataSourceAccount			(void);
CLOUD_EXPORT									DataSourceAccount			(const ServiceName &service, const AccountName &account);
CLOUD_EXPORT									DataSourceAccount			(const ServiceName &service, const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

CLOUD_EXPORT								   ~DataSourceAccount			(void);

CLOUD_EXPORT	void							setDataSourceManager		(DataSourceManager &manager);
CLOUD_EXPORT			DataSourceManager &		getDataSourceManager		(void);

CLOUD_EXPORT	virtual	DataSourceStatus		setAccount					(const ServiceName &service, const AccountName &accountName, const AccountIdentifier &identifier, const AccountKey &key);
			
CLOUD_EXPORT			void					setServiceName				(const ServiceName &name);
CLOUD_EXPORT			const ServiceName &		getServiceName				(void) const;

CLOUD_EXPORT			void					setAccountName				(const AccountName &name);
CLOUD_EXPORT			const AccountName &		getAccountName				(void) const;

CLOUD_EXPORT			void					setAccountIdentifier		(const AccountIdentifier &identifier);
CLOUD_EXPORT	const	AccountIdentifier &		getAccountIdentifier		(void) const;

CLOUD_EXPORT			void					setAccountKey				(const AccountKey &key);
CLOUD_EXPORT	const	AccountKey				getAccountKey				(void) const;

CLOUD_EXPORT	virtual DataSource			*	createDataSource			(void) = 0;
CLOUD_EXPORT			DataSource			*	createDataSource			(DataSource::Name &name);

CLOUD_EXPORT			DataSource			*	getOrCreateDataSource		(DataSource::Name &name, bool *created = nullptr);
CLOUD_EXPORT			DataSource			*	getOrCreateThreadDataSource	(bool *created = nullptr);

CLOUD_EXPORT	virtual DataSourceStatus		destroyDataSource			(DataSource *dataSource) = 0;

CLOUD_EXPORT	DataSourceStatus				uploadSegments				(DataSource &dataSource);
CLOUD_EXPORT	DataSourceStatus				downloadSegments			(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize size);

CLOUD_EXPORT	virtual DataSourceStatus		downloadBlobSync			(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
CLOUD_EXPORT	virtual DataSourceStatus		downloadBlobSync			(const DataSourceURL &blobPath, DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
CLOUD_EXPORT	virtual DataSourceStatus		uploadBlobSync				(const DataSourceURL &blobPath, DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);


CLOUD_EXPORT	virtual DataSourceStatus		setCaching					(DataSourceAccount &cacheAccount, const DataSourceURL &cachingRootPath);

CLOUD_EXPORT	virtual	void					setCacheAccount				(DataSourceAccount *account);
CLOUD_EXPORT	virtual DataSourceAccount	*	getCacheAccount				(void);

CLOUD_EXPORT	void							setPrefixPath				(const DataSourceURL &url);
CLOUD_EXPORT	const	DataSourceURL			getPrefixPath				(void) const;

CLOUD_EXPORT	virtual DataSourceStatus		getFormattedCacheURL		(const DataSourceURL &sourceURL, DataSourceURL &fullCacheURL);

};