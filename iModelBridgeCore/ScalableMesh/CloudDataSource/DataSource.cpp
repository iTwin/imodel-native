#include "stdafx.h"
#include "DataSource.h"
#include "include\DataSource.h"
#include "DataSourceAccount.h"

DataSource::DataSource(DataSourceAccount *sourceAccount)
{
    setService(nullptr);

    setAccount(sourceAccount);

    setTimeout(DataSource::Timeout(100000));

    if (sourceAccount)
    {
        setPrefixPath(sourceAccount->getPrefixPath());
    }
}

DataSource::~DataSource(void)
{
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


