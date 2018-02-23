#include "stdafx.h"
#include "DataSourceLocator.h"
#include "include\DataSourceLocator.h"

DataSourceLocator::DataSourceLocator(void)
{
    setService(nullptr);
    setAccount(nullptr);
    setClientID(nullptr);
}

DataSourceLocator::DataSourceLocator(DataSourceLocator & locator)
{
    *this = locator;
}


DataSourceLocator &DataSourceLocator::operator=(const DataSourceLocator &locator)
    {
    setService(const_cast<DataSourceLocator &>(locator).getService());
    setAccount(locator.getAccount());
    setClientID(const_cast<DataSourceLocator &>(locator).getClientID());

    setPrefixPath(locator.getPrefixPath());
    setSubPath(locator.getSubPath());
    setSegmentName(locator.getSegmentName());

    setMode(locator.getMode());

    return *this;
    }

void DataSourceLocator::getURL(DataSourceURL &url)
{
    url = getPrefixPath();

    url.append(getSubPath());
}

void DataSourceLocator::setService(DataSourceService * newService)
{
    m_service = newService;
}

DataSourceService * DataSourceLocator::getService(void)
{
    return m_service;
}

void DataSourceLocator::setAccount(DataSourceAccount * sourceAccount)
{
    m_account = sourceAccount;
}

DataSourceAccount * DataSourceLocator::getAccount(void) const
{
    return m_account;
}

void DataSourceLocator::setClientID(ClientID client)
    {
    m_clientID = client;
    }

DataSourceLocator::ClientID DataSourceLocator::getClientID(void)
    {
    return m_clientID;
    }

void DataSourceLocator::setPrefixPath(const DataSourceURL & path)
{
    m_prefixPath = path;
}

const DataSourceURL & DataSourceLocator::getPrefixPath(void) const
{
    return m_prefixPath;
}

void DataSourceLocator::setSubPath(const DataSourceURL & path)
{
    m_subPath = path;
}

const DataSourceURL & DataSourceLocator::getSubPath(void) const
{
    return m_subPath;
}

void DataSourceLocator::setSegmentName(const DataSourceURL & name)
{
    m_segmentName = name;
}


const DataSourceURL &DataSourceLocator::getSegmentName(void) const
{
    return m_segmentName;
}

void DataSourceLocator::setMode(DataSourceMode sourceMode)
{
    m_mode = sourceMode;
}

DataSourceMode DataSourceLocator::getMode(void) const
{
    return m_mode;
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