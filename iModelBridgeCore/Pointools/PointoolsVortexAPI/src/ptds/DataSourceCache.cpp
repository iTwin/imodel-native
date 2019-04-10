#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataSourceCache.h>
#include <ptds/DataSourceManager.h>


namespace ptds
{

DataSourceCache::FilePathDataCacheMap	DataSourceCache::dataCacheSet;
std::wstring							DataSourceCache::cacheFolderPath;
bool									DataSourceCache::cachingEnabled						= false;
DataSource::DataSize					DataSourceCache::defaultCachePageSize				= 1024 * 128;
DataSource::DataSize					DataSourceCache::defaultCacheCompletionThreshold	= 1024 * 1024 * 5;
DataSourceCache::CacheFileNameMode		DataSourceCache::cacheFileNameMode					= DataSourceCache::CacheFileNameModeServerPath;


DataSourceCache::DataSourceCache(void)
{
	setCachePageSize(getDefaultCachePageSize());
	setCacheCompletionThreshold(getDefaultCacheCompletionThreshold());
	setCacheFilePath(L"");
	setDataCache(NULL);
															// Initially no data sources
	setDataSourceFullFile(NULL);
	setDataSourceCacheFile(NULL);
	setDataSourceCacheStatusFile(NULL);

	setReadSetEnabled(true);

}


void DataSourceCache::setDataCache(DataCache *cache)
{
	dataCache = cache;
}


DataCache *DataSourceCache::getDataCache(void)
{
	return dataCache;
}


void DataSourceCache::setCacheFilePath(const wchar_t *filePath)
{
	if(filePath != NULL)
	{
		filePathCache = filePath;
	}
}


const wchar_t *DataSourceCache::getCacheFilePath(void)
{
	return filePathCache.c_str();
}


void DataSourceCache::setCachePageSize(DataSize size)
{
	cachePageSize = size;
}


DataSource::DataSize DataSourceCache::getCachePageSize(void)
{
	return cachePageSize;
}

DataSourcePtr DataSourceCache::createNew(const FilePath *path)
{
	if(path == NULL)
		return NULL;

	DataSourceCache *dataSourceCache;

	if(isValidPath(path) == false)
		return NULL;

	if((dataSourceCache = new DataSourceCache()) == NULL)
		return NULL;

	DataCache *dataCache;
	
	if((dataCache = getDataCacheShared(path->path())) == NULL)
		return NULL;
															// Create new data sources for Full file, Cache file and Cache Status file
	if(dataSourceCache->createDataSources(path->path(), dataCache) == false)
		return NULL;

	return dataSourceCache;
}


DataSource::DataSourceType DataSourceCache::getDataSourceType(void)
{
	return DataSourceTypeCache;
}


bool DataSourceCache::openForRead(const FilePath *filePath, bool create)
{
	if(filePath == NULL)
		return false;

	setDataPointer(DATA_SOURCE_DEFAULT_DATA_POINTER);

	if(initializeCache(filePath->path(), getCacheFilePath()) == false)
		return false;

	if(getDataCache() == NULL)
		return false;

	if(getDataCache()->open(filePath->path(), this).isFailed())
		return false;
															// Set state
	setOpenState(DataSourceStateOpenForRead);
															// If cache is completable (and completion is enabled)
	if(getDataCache()->isCacheCompletable())
	{
															// Complete the cache for next open
		if(getDataCache()->completeCache(this).isFailed())
			return false;
	}
	canRead = true;
															// Return OK
	return true;
}


bool DataSourceCache::openForWrite(const FilePath *filepath, bool create)
{
															// Not supported
	return false;
}


bool DataSourceCache::openForReadWrite(const FilePath *filepath, bool create)
{
															// Not supported
	return false;
}


bool DataSourceCache::validHandle(void)
{
	if(getDataSourceFullFile() == NULL || getDataSourceCacheFile() == NULL)
		return false;
															// Check full file
	if(getDataSourceFullFile()->validHandle() == false)
		return false;
															// Check cache file
	if(getDataSourceCacheFile()->validHandle() == false)
		return false;
															// Return OK
	return true;
}


bool DataSourceCache::isValidPath(const FilePath *filepath)
{
	if(filepath == NULL)
		return false;

	PTRMI::URL protocol;
	PTRMI::URL url = filepath->path();

	if(url.isValidURL() == false)
		return false;

	return url.getProtocol(protocol) && ((protocol == PTRMI::URL::PT_PTCE) || (protocol == PTRMI::URL::PT_PTCI));
}


void DataSourceCache::close(void)
{
	if(getDataCache())
	{
															// Flush cache changes to status file
		getDataCache()->manageCacheStatusFileUpdate(this, true);
	}
															// Close and destroy data sources used
	closeDataSources();
															// Record that the cache is closed
	setOpenState(DataSourceStateClosed);
}


bool DataSourceCache::closeAndDelete(void)
{
	// NOTE: To Do
	return true;
}


DataCache *DataSourceCache::createDataCacheShared(const wchar_t *filePath)
{
	if(filePath == NULL)
		return NULL;

	DataCache *newCache;

	if((newCache = new DataCache()) == NULL)
	{
		return NULL;
	}

	dataCacheSet[filePath] = newCache;

	setDataCache(newCache);

	newCache->setCachePageSize(getCachePageSize());

	return newCache;
}


DataCache *DataSourceCache::getDataCacheShared(const wchar_t *filePath)
{
	FilePathDataCacheMap::iterator	it;
	
	if((it = dataCacheSet.find(filePath)) != dataCacheSet.end())
	{
		return it->second;
	}

	return NULL;
}


bool DataSourceCache::initializeCache(const wchar_t *filePathFull, const wchar_t *filePathCache)
{
	if(filePathFull == NULL || filePathCache == NULL)
		return false;

	DataCache *dataCache;
															// Get existing cache	
	if((dataCache = getDataCacheShared(filePathFull)) != NULL)
	{
															// Bind to shared data cache
		setDataCache(dataCache);
															// Data Cache needs to record the cache file path
		if(getCacheFilePath() == NULL && filePathCache)
		{
			dataCache->setCacheFilePath(filePathCache);
		}
															// Create Full, Cache and Cache Status data sources
		if(createDataSources(filePathFull, dataCache) == false)
			return false;

		return true;
	}

															// Create the cache based based on the full file name
	if(dataCache = createDataCacheShared(filePathFull))
	{
															// Data Cache needs to record the cache file path
		dataCache->setCacheFilePath(filePathCache);
															// Set cache page size to default
		dataCache->setCachePageSize(getDefaultCachePageSize());
															// Create Full, Cache and Cache Status data sources
		if(createDataSources(filePathFull, dataCache) == false)
			return false;
															// Return created OK
		return true;
	}
															// Return error
	return false;
}

bool DataSourceCache::getFullFileSubFilePath(PTRMI::URL &fullFile, PTRMI::URL &fullFileSubProtocolFilePath)
{
	PTRMI::URL	fullFileSubProtocol;

	if(getFullFileSubProtocol(fullFile, fullFileSubProtocol) == false)
		return false;
													// Replace existing protocol with sub protocol
	fullFileSubProtocolFilePath = fullFile;

	return fullFileSubProtocolFilePath.setProtocol(fullFileSubProtocol);
}


bool DataSourceCache::getFullFileSubProtocol(PTRMI::URL &fullFile, PTRMI::URL &subProtocol)
{
	PTRMI::URL	protocol;
															// Get main DataSourceCache's full file protocol in URL
	if(fullFile.getProtocol(protocol) == false)
		return false;
															// If Remote External (Ext) based data source cache, use PTRE for full data source
	if(protocol == PTRMI::URL::PT_PTCE)
	{
		subProtocol = PTRMI::URL::PT_PTRE;
		return true;
	}
															// If Remote Internet (TCP/IP) based data source cache, use PTRI for full data source
	if(protocol == PTRMI::URL::PT_PTCI)
	{
		subProtocol = PTRMI::URL::PT_PTRI;
		return true;
	}
															// If local file based data source cache, use no protocol prefix
	if(protocol == PTRMI::URL::PT_PTCF)
	{
		subProtocol = L"";
		return true;
	}
															// DataSourceCache protocol not recognized
	return false;
}

bool DataSourceCache::createDataSources(const wchar_t *filePathFull, DataCache *dataCache)
{
	if(filePathFull == NULL || dataCache == NULL)
		return false;

	const wchar_t *	filePathCache;
	PTRMI::URL		fullFileSubProtocol;
															// Get file path to cache	
	if((filePathCache = dataCache->getCacheFilePath()) == NULL)
		return false;
															// Set up cache status file path
	std::wstring filePathCacheStatus;
	DataCache::getDefaultCacheStatusFilePath(filePathCache, filePathCacheStatus);
	dataCache->setCacheStatusFilePath(filePathCacheStatus.c_str());

	PTRMI::URL	fullFileURL(filePathFull);
	PTRMI::URL	fullFileSubProtocolURL;

	if(getFullFileSubFilePath(fullFileURL, fullFileSubProtocolURL) == false)
		return false;

    ptds::FilePath subUrlPath(fullFileSubProtocolURL.getString().c_str());
	DataSource *dataSourceFullFile			= dataSourceManager.createDataSource(&subUrlPath);

    ptds::FilePath cacheFilePath(filePathCache);
	DataSource *dataSourceCacheFile			= dataSourceManager.createDataSource(&cacheFilePath);

    ptds::FilePath CacheStatusPath(filePathCacheStatus.c_str());
	DataSource *dataSourceCacheStatusFile	= dataSourceManager.createDataSource(&CacheStatusPath);

															// If a data source wasn't created properly, delete all others and exit
	if(dataSourceFullFile == NULL || dataSourceCacheFile == NULL || dataSourceCacheStatusFile == NULL)
	{
		if(dataSourceFullFile)
			dataSourceManager.deleteDataSource(dataSourceFullFile);

		if(dataSourceCacheFile)
			dataSourceManager.deleteDataSource(dataSourceCacheFile);

		if(dataSourceCacheStatusFile)
			dataSourceManager.deleteDataSource(dataSourceCacheStatusFile);

		return false;
	}
															// Set data sources
	setDataSourceFullFile(dataSourceFullFile);
	setDataSourceCacheFile(dataSourceCacheFile);
	setDataSourceCacheStatusFile(dataSourceCacheStatusFile);

															// Return OK
	return true;
}


void DataSourceCache::closeDataSources(void)
{
	if(getDataSourceFullFile())
		getDataSourceFullFile()->close();

	if(getDataSourceCacheFile())
		getDataSourceCacheFile()->close();

	if(getDataSourceCacheStatusFile())
		getDataSourceCacheStatusFile()->close();

/*
															// Close and destroy data sources
	dataSourceManager.close(getDataSourceFullFile());
	dataSourceManager.close(getDataSourceCacheFile());
	dataSourceManager.close(getDataSourceCacheStatusFile());

	setDataSourceFullFile(NULL);
	setDataSourceCacheFile(NULL);
	setDataSourceCacheStatusFile(NULL);
*/

}


void DataSourceCache::destroy(void)
{
	if(getDataSourceFullFile())
	{
		getDataSourceFullFile()->destroy();
		setDataSourceFullFile(NULL);
	}

	if(getDataSourceCacheFile())
	{
		getDataSourceCacheFile()->destroy();
		setDataSourceCacheFile(NULL);
	}

	if(getDataSourceCacheStatusFile())
	{
		getDataSourceCacheStatusFile()->destroy();
		setDataSourceCacheStatusFile(NULL);
	}

}

DataSource::Size DataSourceCache::readBytes(Data *buffer, Size numBytes)
{
	if(buffer == NULL || getDataCache() == NULL || numBytes == 0)
		return 0;

	DataSize	numRead;


	beginRead(numBytes);
															// Construct ReadSet in this DataSource if defined
	if(isReadSetDefined())
	{
															// Add item to the read set
		numRead = addReadSetItem(buffer, getDataPointer(), numBytes);
	}
	else
	{

															// Pass read through to cache
		getDataCache()->readBytes(getDataPointer(), buffer, numBytes, this, numRead);
	}

	endRead(numBytes);


	advanceDataPointer(numRead);

	return numRead;
}


DataSource::Size DataSourceCache::readBytesReadSet(Data *buffer, DataSourceReadSet *readSet)
{
	if(getDataCache())
	{
		return getDataCache()->readBytesReadSet(*this, buffer, *readSet);
	}

	return 0;
}


DataSource::Size DataSourceCache::writeBytes(const Data *buffer, Size number_bytes)
{
															// Not Supported
	return 0;
}


void DataSourceCache::setDataSourceFullFile(DataSource * initDataSourceFullFile)
{
	dataSourceFullFile = initDataSourceFullFile;
}


DataSource *DataSourceCache::getDataSourceFullFile(void)
{
	return dataSourceFullFile;
}


void DataSourceCache::setDataSourceCacheFile(DataSource * initDataSourceCacheFile)
{
	dataSourceCacheFile = initDataSourceCacheFile;
}


DataSource *DataSourceCache::getDataSourceCacheFile(void)
{
	return dataSourceCacheFile;
}


void DataSourceCache::setDataSourceCacheStatusFile(DataSource * initDataSourceCacheStatusFile)
{
	dataSourceCacheStatusFile = initDataSourceCacheStatusFile;
}


DataSource *DataSourceCache::getDataSourceCacheStatusFile(void)
{
	return dataSourceCacheStatusFile;
}


DataSource::DataSize DataSourceCache::getFileSize(void)
{
	DataSource *dataSource;

	if(dataSource = getDataSourceFullFile())
	{
		return dataSource->getFileSize();
	}

	return 0;
}


bool DataSourceCache::movePointerBy(DataPointer number_bytes)
{
	advanceDataPointer(number_bytes);

	return true;
}


bool DataSourceCache::movePointerTo(DataPointer number_bytes)
{
	setDataPointer(number_bytes);

	return true;
}


DataSource *DataSourceCache::getDataSourceForState(DataCache::CachePageState state)
{
	if(state == DataCache::PageStateResident)
		return getDataSourceCacheFile();

	if(state == DataCache::PageStateNonResident)
		return getDataSourceFullFile();

	return NULL;
}


bool DataSourceCache::setCachingEnabled(bool enabled)
{
	cachingEnabled = enabled;

	return true;
}


bool DataSourceCache::getCachingEnabled(void)
{
	return cachingEnabled;
}


bool DataSourceCache::setCacheFolderPath(const wchar_t *path)
{
	cacheFolderPath = path;

	return true;
}


const wchar_t *DataSourceCache::getCacheFolderPath(void)
{
	return cacheFolderPath.c_str();
}


bool DataSourceCache::setDefaultCachePageSize(DataSize size)
{
	if(defaultCachePageSize > 0)
	{
		defaultCachePageSize = size;
		return true;
	}

	return false;
}


DataSource::DataSize DataSourceCache::getDefaultCachePageSize(void)
{
	return defaultCachePageSize;
}


bool DataSourceCache::setCacheCompletionThreshold(DataSize threshold)
{
	cacheCompletionThreshold = threshold;

	return true;
}


DataSource::DataSize DataSourceCache::getCacheCompletionThreshold(void)
{
	return cacheCompletionThreshold;
}


bool DataSourceCache::setAutoCacheFilePath(const wchar_t *serverFilePath, PTRMI::GUID &guid)
{
	switch(getCacheFileNameMode())
	{
															// Generate GUID filename 
	case CacheFileNameModeGUID:
		return setAutoCacheFilePathGUID(getCacheFolderPath(), guid);
															// Generate server file path based cache path
	case CacheFileNameModeServerPath:
	default:
		return setAutoCacheFilePathServerFilePath(getCacheFolderPath(), serverFilePath);
	}
}


bool DataSourceCache::setAutoCacheFilePathGUID(const wchar_t *cacheFolder, PTRMI::GUID &guid)
{
	if(cacheFolder == NULL || guid.isValidGUID() == false)
		return false;

	std::wstring	guidString;
	std::wstring	filePath;
															// Generate string version of GUID for file name
	guid.getHexString(guidString);
	if(guidString.length() == 0)
		return false;
															// Construct cache file path from cache folder and GUID string and standard postfix
	filePath = cacheFolder;
	filePath.append(DATA_SOURCE_CACHE_FILE_SEPARATOR);
	filePath.append(guidString);
	filePath.append(DATA_SOURCE_CACHE_FILE_POSTFIX);
															// Set the cache file path
	setCacheFilePath(filePath.c_str());

	return true;
}


bool DataSourceCache::setAutoCacheFilePathServerFilePath(const wchar_t *cacheFolder, const wchar_t *serverFilePath)
{
	if(cacheFolder == NULL || serverFilePath == NULL)
		return false;

	std::wstring	guidString;
	std::wstring	filePath;

	PTRMI::URL	path(serverFilePath);
	PTRMI::URL	protocol, hostAddress, object;

	path.split(protocol, hostAddress, object);

	std::wstring serverPath = object.getString();
															// Turn path into a file name by replacing path separators
	std::replace(serverPath.begin(), serverPath.end(), '/', '_' );
	std::replace(serverPath.begin(), serverPath.end(), '\\', '_' );
	std::replace(serverPath.begin(), serverPath.end(), ':', '_' );

															// Construct cache file path from cache folder and server path string and standard postfix
	filePath = cacheFolder;
	filePath.append(DATA_SOURCE_CACHE_FILE_SEPARATOR);
	filePath.append(serverPath);
	filePath.append(DATA_SOURCE_CACHE_FILE_POSTFIX);
															// Set the cache file path
	setCacheFilePath(filePath.c_str());

	return true;
}


bool DataSourceCache::setDefaultCacheCompletionThreshold(DataSize size)
{
	defaultCacheCompletionThreshold = size;

	return true;
}


DataSource::DataSize DataSourceCache::getDefaultCacheCompletionThreshold(void)
{
	return defaultCacheCompletionThreshold;
}


bool DataSourceCache::setCacheFileNameMode(CacheFileNameMode mode)
{
	if(mode < CacheFileNameModeCount)
	{
		cacheFileNameMode = mode;
		return true;
	}

	return false;
}


DataSourceCache::CacheFileNameMode DataSourceCache::getCacheFileNameMode(void)
{
	return cacheFileNameMode;
}


DataSource::DataSourceForm DataSourceCache::getDataSourceForm(void)
{
	DataSourcePtr	dataSourceFullFile;
															// Default to the full file's data source form
	if((dataSourceFullFile = getDataSourceFullFile()) != NULL)
	{
		return dataSourceFullFile->getDataSourceForm();
	}
															// Return not defined
	return DataSource::DataSourceFormNULL;
}


unsigned int DataSourceCache::getBudgetParallelRead(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed)
{
	if(getDataCache())
	{
		return getDataCache()->getBudgetParallelRead(budget, readSet, budgetUsed);
	}

	return 0;
}


bool DataSourceCache::beginReadSet(DataSourceReadSet *readSet)
{
															// If readSet specified and cache exists
	if(readSet && getDataCache())
	{
															// Begin the read set
		getDataCache()->beginReadSet();
															// General data source begin
		if(DataSource::beginReadSet(readSet))
		{
			return true;
		}
															// Error occurred, so end read set immediately
		getDataCache()->endReadSet();
	}
															// Error occurred, so return false
	return false;
}


bool DataSourceCache::endReadSet(DataSourceReadSet *readSet)
{
															// If readSet specified and cache exists
	if(readSet && getDataCache())
	{
															// End the read set
		getDataCache()->endReadSet();
															// General data source end
		if(DataSource::endReadSet(readSet))
		{
			return true;
		}
	}
															// Error occurred, so return false
	return false;
}


}
