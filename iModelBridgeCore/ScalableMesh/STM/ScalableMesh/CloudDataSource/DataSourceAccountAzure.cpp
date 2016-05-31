#include "stdafx.h"
#include "DataSourceAccountAzure.h"
#include "DataSourceAzure.h"
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>


DataSourceAccountAzure::DataSourceAccountAzure(const ServiceName & name, const AccountIdentifier & identifier, const AccountKey & key)
{
	setAccount(name, identifier, key);
}

DataSourceAccountAzure::AzureConnectionString DataSourceAccountAzure::createConnectionString(AccountIdentifier identifier, AccountKey key)
{
	AzureConnectionString	cs;

	cs += std::wstring(L"DefaultEndpointsProtocol=https;");
	cs += std::wstring(L"AccountName=") + identifier + L";";
	cs += L"AccountKey=" + key;

	return cs;
}

DataSourceStatus DataSourceAccountAzure::setConnectionString(const AzureConnectionString string)
{
	if (string.length() == 0)
		return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);

	connectionString = string;

	return DataSourceStatus();
}

const DataSourceAccountAzure::AzureConnectionString & DataSourceAccountAzure::getConnectionString(void)
{
	return connectionString;
}

void DataSourceAccountAzure::setStorageAccount(const AzureStorageAccount & account)
{
	storageAccount = account;
}

const DataSourceAccountAzure::AzureStorageAccount & DataSourceAccountAzure::getStorageAccount(void)
{
	return storageAccount;
}

void DataSourceAccountAzure::setBlobClient(const AzureBlobClient &client)
{
	blobClient = client;
}

DataSourceAccountAzure::AzureBlobClient &DataSourceAccountAzure::getBlobClient(void)
{
	return blobClient;
}

void DataSourceAccountAzure::setContainer(const AzureContainer & newContainer)
{
	container = newContainer;
}

DataSourceAccountAzure::AzureContainer & DataSourceAccountAzure::getContainer(void)
{
	return container;
}

DataSource * DataSourceAccountAzure::createDataSource(void)
{
	return new DataSourceAzure(this);
}


DataSourceStatus DataSourceAccountAzure::destroyDataSource(DataSource *dataSource)
{
	if (dataSource)
	{
		delete dataSource;

		return DataSourceStatus();
	}

	return DataSourceStatus(DataSourceStatus::Status_Error);
}

DataSourceStatus DataSourceAccountAzure::initializeContainer(const DataSourceURL & containerName, DataSourceMode mode)
{
	if (getContainer().is_valid())
		return DataSourceStatus();

	setContainer(getBlobClient().get_container_reference(containerName));

	if (mode == DataSourceMode::DataSourceMode_Write)
	{
		getContainer().create_if_not_exists();
	}

	return DataSourceStatus();
}


DataSourceStatus DataSourceAccountAzure::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
{
	if (account.length() == 0 || identifier.length() == 0 || key.length() == 0)
		return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
															// Set details in base class
	DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceAzure"), account, identifier, key);
															// Calculate and store the Azure connection string
	setConnectionString(createConnectionString(identifier, key));

	if (getConnectionString().length() == 0)
		return DataSourceStatus(DataSourceStatus::Status_Error);
															// Create storage account reference
	setStorageAccount(AzureStorageAccount::parse(getConnectionString()));
	if (getStorageAccount().is_initialized() == false)
		return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Initialize_Subsystem);

	setBlobClient(storageAccount.create_cloud_blob_client());

	return DataSourceStatus();
}


DataSourceStatus DataSourceAccountAzure::download(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
	return downloadBlobSync(dataSource.getSubPath(), dest, readSize, destSize);
}

DataSourceStatus DataSourceAccountAzure::downloadBlobSync(const DataSourceURL & blobPath, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size)
{
	if(getContainer().is_valid() == false)
		return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);

	azure::storage::cloud_block_blob	blockBlob = getContainer().get_block_blob_reference(blobPath);
	if (blockBlob.is_valid() == false)
		return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);


	std::streampos p;

	try
	{
		concurrency::streams::producer_consumer_buffer<unsigned char>	pcb;

		concurrency::streams::ostream stream = pcb.create_ostream();

		blockBlob.download_to_stream(stream);

		p = stream.tell();

		readSize = p;

		stream.close().wait();

		pcb.getn(dest, size).wait();
	}
	catch (...)
	{
		return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
	}

	return DataSourceStatus();
}

DataSourceStatus DataSourceAccountAzure::uploadBlobSync(const DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
{

	azure::storage::cloud_block_blob	blockBlob = getContainer().get_block_blob_reference(blobPath);
	if (blockBlob.is_valid() == false)
		return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);

	try
	{
		concurrency::streams::producer_consumer_buffer<unsigned char>	pcb;

		pcb.putn_nocopy(source, size).wait();

		concurrency::streams::istream stream = pcb.create_istream();

		blockBlob.upload_from_stream(stream, size);

		stream.close().wait();
	}
	catch (...)
	{
		return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
	}


	return DataSourceStatus();
}
