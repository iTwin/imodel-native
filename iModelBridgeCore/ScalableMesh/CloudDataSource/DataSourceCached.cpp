/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "DataSourceCached.h"
#include "DataSourceAccount.h"
#include <Bentley/BeFileName.h>

#ifndef NDEBUG
#ifdef _WIN32
void SetThreadName(DWORD dwThreadID, char const* threadName)
{

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
        {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {}

}
#endif
#endif

DataSourceCached::DataSourceCached(DataSourceAccount * account, const SessionName &session) : Super(account, session)
{
                                                            // Initially caching is disabled by default
    setCachingEnabled(account->getCacheAccount() != nullptr);
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
    DataSource           *  dataSource;
    DataSourceStatus        statusNotFound(DataSourceStatus::Status_Not_Found);
    DataSourceAccount   *   cacheAccount;

    (void) destSize;

    if ((dataSource = getCacheDataSource()) == nullptr && getAccount())
    {
        if (cacheAccount = getAccount()->getCacheAccount())
            {
            DataSourceName dataSourceName(getName());
            dataSourceName += L"-Cache";

            DataSource::SessionName   sessionName (getSessionName());

            if ((dataSource = cacheAccount->createDataSource(dataSourceName, sessionName)) == nullptr)
                {
                return DataSourceStatus(DataSourceStatus::Status_Error);
                }

            setCacheDataSource(dataSource);
            }
    }

    if (dataSource == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Create_DataSource);

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

DataSourceStatus DataSourceCached::readFromCache(std::vector<DataSourceBuffer::BufferData>& dest)
    {
    DataSource           *  dataSource;
    DataSourceStatus        statusNotFound(DataSourceStatus::Status_Not_Found);
    DataSourceAccount   *   cacheAccount;

    if ((dataSource = getCacheDataSource()) == nullptr && getAccount())
        {
        if (cacheAccount = getAccount()->getCacheAccount())
            {
            DataSourceName dataSourceName(getName());
            dataSourceName += L"-Cache";

            DataSource::SessionName   sessionName(getSessionName());

            if ((dataSource = cacheAccount->createDataSource(dataSourceName, sessionName)) == nullptr)
                {
                return DataSourceStatus(DataSourceStatus::Status_Error);
                }

            setCacheDataSource(dataSource);
            }
        }

    if (dataSource == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Create_DataSource);

    if ((dataSource->open(getCacheURL(), DataSourceMode_Read)).isFailed())
        return statusNotFound;

    if ((dataSource->read(dest)).isFailed())
        return statusNotFound;

    if ((dataSource->close()).isFailed())
        return statusNotFound;

    return DataSourceStatus();

    }

bool IsUrl(WCharCP filename)
{
	return NULL != filename && (0 == wcsncmp(L"http:", filename, 5) || 0 == wcsncmp(L"https:", filename, 6));
}

CacheWriter::Ptr CacheWriter::s_cacheWriter = nullptr;

CacheWriter::Ptr CacheWriter::GetCacheWriter()
    { 
    static std::mutex cacheWriterMutex;
    std::lock_guard<std::mutex> lck(cacheWriterMutex);
    if (!s_cacheWriter.IsValid()) s_cacheWriter = new CacheWriter(); 
    return s_cacheWriter; 
    }

void CacheWriter::ShutdownCacheWriter()
    {
    if (s_cacheWriter.IsValid())
        {
        s_cacheWriter->Shutdown();
        s_cacheWriter = nullptr;
        }
    }

DataSourceStatus DataSourceCached::writeToCache(DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize size)
{
    DataSource            *    dataSource;
    DataSourceStatus        statusErrorWrite(DataSourceStatus::Status_Error_Write);
    
    dataSource = getCacheDataSource();
    if (dataSource)
    {
        DataSourceURL url;
        dataSource->getURL(url);
        CacheWriter::GetCacheWriter()->push(new CacheWriter::CacheData(url, dest, size));
    }

    return DataSourceStatus();
}

DataSourceStatus DataSourceCached::writeToCache(std::vector<DataSourceBuffer::BufferData>& dest)
    {
    DataSource            *    dataSource;
    DataSourceStatus        statusErrorWrite(DataSourceStatus::Status_Error_Write);

    dataSource = getCacheDataSource();
    if (dataSource)
        {
        DataSourceURL url;
        dataSource->getURL(url);
        CacheWriter::GetCacheWriter()->push(new CacheWriter::CacheData(url, dest));
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

void DataSourceCached::setForceWriteToCache(void)
    {
    writeCache = true;
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
            DataSourceURL url = sourceURL;
            if (IsUrl(url.c_str()))
                {
                auto directoryPath = DataSourceURL(BeFileName::GetDirectoryName(url.c_str()).c_str());
                directoryPath.findAndReplace(std::wstring(DATA_SOURCE_URL_SEPARATOR_STR), std::wstring(L"-"));
                directoryPath.findAndReplace(std::wstring(DATA_SOURCE_URL_WINDOWS_DEVICE_SEPARATOR_STR), std::wstring(L"-"));

                auto filename = BeFileName::GetFileNameAndExtension(url.c_str());
                url = directoryPath + L"/" + DataSourceURL(filename.c_str());
                }
            url = getSessionName().getSessionKey() + L"/" + url;
                                                            // Generate the full URL of the cache file
            if ((status = getAccount()->getFormattedCacheURL(url, fullCacheURL)).isFailed())
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
            setWriteToCache(false);
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
            m_isFromCache = true;
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
            return DataSourceStatus();                      // Not a big deal if writing to cache has failed
    }
                                                            // Return status
    return status;
}

DataSourceStatus DataSourceCached::read(std::vector<Buffer>& dest)
    {
    DataSourceStatus    status;
    // If caching is enabled
    if (getCachingEnabled())
        {
        // Try reading from the cache
        if (readFromCache(dest).isOK())
            {
            m_isFromCache = true;
            // If read, return OK
            return status;
            }
        // If not read, Flag to write to cache on next read
        setWriteToCache(true);
        }
    // Get Superclass to read
    if ((status = Super::read(dest)).isFailed())
        return status;
    // If cache needs writing
    if (getWriteToCache())
        {
        // Write to the cache
        setWriteToCache(false);
        // Write to cache [Note: This could be on separate thread in future]
        if ((status = writeToCache(dest)).isFailed())
            return DataSourceStatus();                      // Not a big deal if writing to cache has failed
        }
    // Return status
    return status;
    }

DataSourceStatus DataSourceCached::write(const Buffer * source, DataSize size)
{

    // Write to cache if immediate cache writes are enabled

    return Super::write(source, size);
}

