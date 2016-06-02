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
	return Super::open(sourceURL, sourceMode);
}


DataSourceStatus DataSourceAzure::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size)
{
															// Pass to superclass for read (Note: Real reads are down through download calls)
	return Super::read(dest, destSize, readSize, size);
}

DataSourceStatus DataSourceAzure::close(void)
{
	return Super::close();
}

DataSourceAccountAzure * DataSourceAzure::getAccountAzure(void)
{
	return dynamic_cast<DataSourceAccountAzure *>(getAccount());
}

