#include "stdafx.h"
#include "DataSource.h"


DataSource::DataSource(DataSourceAccount *sourceAccount)
{
	setService(nullptr);

	setAccount(sourceAccount);

	setTimeout(DataSource::Timeout(0));
}

DataSource::~DataSource(void)
{
}

bool DataSource::isValid(void)
{
	return false;
}

void DataSource::setTimeout(Timeout timeMilliseconds)
{
	timeout = timeMilliseconds;
}


DataSource::Timeout DataSource::getTimeout(void)
{
	return timeout;
}


void DataSource::setCachingEnabled(bool enabled)
{
	(void) enabled;

	DataSourceStatus status(DataSourceStatus::Status_Error_Not_Supported);
}

bool DataSource::getCachingEnabled(void)
{
	DataSourceStatus status(DataSourceStatus::Status_Error_Not_Supported);
	return false;
}


