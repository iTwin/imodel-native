#include "stdafx.h"
#include "DataSourceLocator.h"

DataSourceLocator::DataSourceLocator(void)
{
	setService(nullptr);
	setAccount(nullptr);
}

DataSourceLocator::DataSourceLocator(DataSourceLocator & locator)
{
	url			= locator.getURL();
	service		= locator.getService();
	account		= locator.getAccount();
}

void DataSourceLocator::setURL(DataSourceURL & newURL)
{
	url = newURL;
}

const DataSourceURL & DataSourceLocator::getURL(void)
{
	return url;
}

void DataSourceLocator::setService(DataSourceService * newService)
{
	service = newService;
}

DataSourceService * DataSourceLocator::getService(void)
{
	return service;
}

void DataSourceLocator::setAccount(DataSourceAccount * sourceAccount)
{
	account = sourceAccount;
}

DataSourceAccount * DataSourceLocator::getAccount(void) const
{
	return account;
}

void DataSourceLocator::setContainerName(DataSourceURL & name)
{
	containerName = name;
}

const DataSourceURL & DataSourceLocator::getContainerName(void) const
{
	return containerName;
}

void DataSourceLocator::setSubPath(DataSourceURL & path)
{
	subPath = path;
}

const DataSourceURL &DataSourceLocator::getSubPath(void) const
{
	return subPath;
}

void DataSourceLocator::setSegmentName(DataSourceURL & name)
{
	segmentName = name;
}


const DataSourceURL &DataSourceLocator::getSegmentName(void) const
{
	return segmentName;
}

void DataSourceLocator::setMode(DataSourceMode sourceMode)
{
	mode = sourceMode;
}

DataSourceMode DataSourceLocator::getMode(void) const
{
	return mode;
}


void DataSourceLocator::getFullSubPath(DataSourceURL &dest) const
{
	dest = getSubPath();

	if (getSegmentName().size() > 0)
	{
		dest += getSegmentName();
	}
}