/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "DataSource.h"
#include "include/DataSource.h"
#include "DataSourceAccount.h"

DataSource::DataSource(DataSourceAccount *sourceAccount, const SessionName &session)
{
    setService(nullptr);

    setAccount(sourceAccount);

    setSessionName(session);

    setTimeout(DataSource::Timeout(0));

    if (sourceAccount)
    {
        setPrefixPath(sourceAccount->getPrefixPath());
    }
}

DataSource::~DataSource(void)
{
}


bool DataSource::destroyAll(void)
{
    close();

    return true;
}


DataSourceStatus DataSource::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
    setSubPath(sourceURL);

    setMode(sourceMode);

    return DataSourceStatus();
}

bool DataSource::isValid(void)
{
    return false;
}

bool DataSource::isEmpty(void)
    {
    return true;
    }

bool DataSource::isFromCache(void)
    {
    return m_isFromCache;
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

DataSource * DataSource::getCacheDataSource(void)
{
                                                            // By default, DataSource types do not have cache DataSources
    return nullptr;
}


