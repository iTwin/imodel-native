#include "stdafx.h"
#include "DataSourceCached.h"
#include "DataSourceAccount.h"


DataSourceCached::DataSourceCached(DataSourceAccount * account) : Super(account)
{
                                                            // Initially caching is disabled by default
    setCachingEnabled(false);
                                                            // NEEDS_WORK_SM - Write to cache by default even if caching is disabled?
    setWriteToCache(false);
                                                            // Initially no cache DataSource
    setCacheDataSource(nullptr);
}

void DataSourceCached::setCachingEnabled(bool enabled)
{
    cachingEnabled = enabled;
}

bool DataSourceCached::getCachingEnabled(void)
{
    return cachingEnabled;
}

bool DataSourceCached::isCached(void)
{
    return false;
}

DataSourceStatus DataSourceCached::readFromCache(DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size)
{
    DataSource                        *    dataSource;
    DataSourceStatus                    statusNotFound(DataSourceStatus::Status_Not_Found);

    (void) destSize;

    if ((dataSource = getCacheDataSource()) == nullptr && getAccount())
    {
        if (getAccount()->getCacheAccount())
        {
            if ((dataSource = getAccount()->getCacheAccount()->createDataSource()) == nullptr)
            {
                return DataSourceStatus(DataSourceStatus::Status_Error);
            }

            setCacheDataSource(dataSource);
        }
    }

    if ((dataSource->open(getCacheURL(), DataSourceMode_Read)).isFailed())
        return statusNotFound;

    if ((dataSource->read(dest, destSize, readSize, size)).isFailed())
        return statusNotFound;

    if ((dataSource->close()).isFailed())
        return statusNotFound;

    if (size > 0 && readSize != size)
        return DataSourceStatus(DataSourceStatus::Status_Error_EOF);

    return DataSourceStatus();
}

DataSourceStatus DataSourceCached::writeToCache(DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize size)
{
    DataSource            *    dataSource;
    DataSourceStatus        statusErrorWrite(DataSourceStatus::Status_Error_Write);
    
    dataSource = getCacheDataSource();
    if (dataSource)
    {
        if ((dataSource->open(getCacheURL(), DataSourceMode_Write)).isFailed())
            return statusErrorWrite;

        if ((dataSource->write(dest, size)).isFailed())
            return statusErrorWrite;

        if ((dataSource->close()).isFailed())
            return statusErrorWrite;
    }

    return DataSourceStatus();
}

void DataSourceCached::setWriteToCache(bool write)
{
    writeCache = write;
}

bool DataSourceCached::getWriteToCache(void)
{


    return writeCache;
}

void DataSourceCached::setCacheURL(const DataSourceURL &url)
{
    cacheURL = url;
}

const DataSourceURL &DataSourceCached::getCacheURL(void)
{
    return cacheURL;
}

void DataSourceCached::setCacheDataSource(DataSource * source)
{
    cacheDataSource = source;
}

DataSource * DataSourceCached::getCacheDataSource(void)
{
    return cacheDataSource;
}

DataSourceStatus DataSourceCached::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
    DataSourceStatus    status;
                                                            // If caching is enabled
    if (getCachingEnabled())
    {
        DataSourceURL    fullCacheURL;
                                                            // Get this DataSource's account
        if (getAccount())
        {
                                                            // Generate the full URL of the cache file
            if ((status = getAccount()->getFormattedCacheURL(sourceURL, fullCacheURL)).isFailed())
                return status;
                                                            // Set the full cache URL
            setCacheURL(fullCacheURL);
        }
        else
        {
                                                            // Account should be set, so return error
            return DataSourceStatus(DataSourceStatus::Status_Error_Account_Not_Found);
        }
                                                            // If present in cache
    }

    return Super::open(sourceURL, sourceMode);
}

DataSourceStatus DataSourceCached::close(void)
{
    DataSourceStatus    status;

    if ((status = Super::close()).isFailed())
        return status;
                                                            // If caching is turned on
    if (getCachingEnabled() && getMode() == DataSourceMode_Read)
    {
                                                            // If a cache file is required (not cached already)
        if (getWriteToCache() && getBuffer())
        {
            if ((status = writeToCache(getBuffer()->getExternalBuffer(), getBuffer()->getExternalBufferSize())).isFailed())
                return status;
        }
    }
                                                            // Return OK
    return status;
}


DataSourceStatus DataSourceCached::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size)
{
    DataSourceStatus    status;
                                                            // If caching is enabled
    if(getCachingEnabled())
    {
                                                            // Try reading from the cache
        if (readFromCache(dest, destSize, readSize, size).isOK())
        {
                                                            // If read, return OK
            return status;
        }
                                                            // If not read, Flag to write to cache on next read
        setWriteToCache(true);
    }
                                                            // Get Superclass to read
    if ((status = Super::read(dest, destSize, readSize, size)).isFailed())
        return status;
                                                            // If cache needs writing
    if (getWriteToCache())
    {
                                                            // Write to the cache
        setWriteToCache(false);
                                                            // Write to cache [Note: This could be on separate thread in future]
        if ((status = writeToCache(dest, readSize)).isFailed())
            return status;
    }
                                                            // Return status
    return status;
}

DataSourceStatus DataSourceCached::write(Buffer * source, DataSize size)
{

    // Write to cache if immediate cache writes are enabled

    return Super::write(source, size);
}

