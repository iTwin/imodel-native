#pragma once
#include "DataSourceDefs.h"
#include <was/storage_account.h>
#include <was/blob.h>
#include <string>

#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceAccountCached.h"
#include "DataSourceBuffer.h"
#include "DataSourceMode.h"

class DataSourceAccountAzure : public DataSourceAccountCached
{


protected:

	typedef std::wstring								AzureConnectionString;
	typedef azure::storage::cloud_storage_account		AzureStorageAccount;
	typedef azure::storage::cloud_blob_client			AzureBlobClient;
	typedef azure::storage::cloud_blob_container		AzureContainer;

protected:

	AzureStorageAccount					storageAccount;
	AzureBlobClient						blobClient;
	AzureConnectionString				connectionString;

protected:

	AzureConnectionString				createConnectionString			(AccountIdentifier identifier, AccountKey key);

	DataSourceStatus					setConnectionString				(const AzureConnectionString string);
	const AzureConnectionString		&	getConnectionString				(void);

	void								setStorageAccount				(const AzureStorageAccount &account);
	const AzureStorageAccount		&	getStorageAccount				(void);

	void								setBlobClient					(const AzureBlobClient &client);
	AzureBlobClient					&	getBlobClient					(void);

public:

CLOUD_EXPORT										DataSourceAccountAzure			(const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

CLOUD_EXPORT		DataSourceStatus				setAccount						(const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);

CLOUD_EXPORT		DataSource					*	createDataSource				(void);
CLOUD_EXPORT		DataSourceStatus				destroyDataSource				(DataSource *dataSource);

CLOUD_EXPORT		AzureContainer					initializeContainer				(const DataSourceURL &containerName, DataSourceMode mode);

CLOUD_EXPORT		DataSourceStatus				downloadBlobSync				(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
CLOUD_EXPORT		DataSourceStatus				downloadBlobSync				(const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
CLOUD_EXPORT		DataSourceStatus				uploadBlobSync					(const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
};
