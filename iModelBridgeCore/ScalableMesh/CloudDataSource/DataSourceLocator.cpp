#include "stdafx.h"
#include "DataSourceLocator.h"
#include "include\DataSourceLocator.h"

DataSourceLocator::DataSourceLocator(void)
{
    setService(nullptr);
    setAccount(nullptr);
}

DataSourceLocator::DataSourceLocator(DataSourceLocator & locator)
{
    setPrefixPath(locator.getPrefixPath());
    setSubPath(locator.getSubPath());
    setService(locator.getService());
    setAccount(locator.getAccount());
}


void DataSourceLocator::getURL(DataSourceURL &url)
{
    url = getPrefixPath();

    url.append(getSubPath());
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

void DataSourceLocator::setPrefixPath(const DataSourceURL & path)
{
    prefixPath = path;
}

const DataSourceURL & DataSourceLocator::getPrefixPath(void)
{
    return prefixPath;
}

void DataSourceLocator::setSubPath(const DataSourceURL & path)
{
    subPath = path;
}

const DataSourceURL & DataSourceLocator::getSubPath(void)
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

/*
void DataSourceLocator::getFullSubPath(DataSourceURL &dest) const
{
    dest = getSubPath();

    if (getSegmentName().size() > 0)
    {
        dest += getSegmentName();
    }
}
*/