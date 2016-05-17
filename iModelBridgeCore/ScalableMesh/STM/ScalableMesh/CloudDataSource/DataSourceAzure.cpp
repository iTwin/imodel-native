#include "stdafx.h"
#include "DataSourceAzure.h"
#include "DataSourceAccountAzure.h"


DataSourceAzure::DataSourceAzure(DataSourceAccount * sourceAccount) : DataSourceCloud(sourceAccount)
{
	setBuffer(nullptr);
}

DataSourceAzure::~DataSourceAzure(void)
{

}

DataSourceStatus DataSourceAzure::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
	DataSourceStatus			status;
	DataSourceURL				containerName;
	DataSourceURL				blobPathName;
	DataSourceAccountAzure *	azureAccount;


	setMode(sourceMode);
															// Attempt to open in superclasses
	if ((status = Super::open(sourceURL, sourceMode)).isFailed())
		return status;

															// Get account
	if((azureAccount = dynamic_cast<DataSourceAccountAzure *>(getAccount())) == nullptr)
		return DataSourceStatus(DataSourceStatus::Status_Error_Not_Initialized);

															// Get first directory as Container and the remainder as the blob's virtual path
	if ((status = sourceURL.getContainerAndBlob(containerName, blobPathName)).isFailed())
		return status;

															// Set the name of the root container
	setContainerName(containerName);
															// Set blob's name within the root container
	setSubPath(blobPathName);
															// Make sure container exists
	status = azureAccount->initializeContainer(containerName, DataSourceMode(DataSourceMode_Write));

	return status;
}

DataSourceStatus DataSourceAzure::close(void)
{
	return Super::close();
}

DataSourceAccountAzure * DataSourceAzure::getAccountAzure(void)
{
	return dynamic_cast<DataSourceAccountAzure *>(getAccount());
}

