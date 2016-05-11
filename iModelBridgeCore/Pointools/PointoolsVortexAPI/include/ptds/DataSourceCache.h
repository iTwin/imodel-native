#pragma once

#include <ptds/DataSource.h>
#include <ptds/DataCache.h>

#define DATA_SOURCE_CACHE_FILE_POSTFIX			L".podc"		// Standard Postfix for cache files
#define DATA_SOURCE_CACHE_FILE_SEPARATOR		L"/"			

namespace ptds
{

class DataSourceCache : public DataSource
{

public:

	enum CacheFileNameMode
	{
		CacheFileNameModeGUID		= 1,			// Must stay as 1
		CacheFileNameModeServerPath	= 2,			// Must stay as 2

		CacheFileNameModeCount		= 3
	};

protected:

	typedef std::map<std::wstring, DataCache *>	FilePathDataCacheMap;

protected:

	static FilePathDataCacheMap	dataCacheSet;

	DataCache				*	dataCache;
	std::wstring				filePathCache;
	DataSize					cachePageSize;

	DataSource				*	dataSourceFullFile;
	DataSource				*	dataSourceCacheFile;
	DataSource				*	dataSourceCacheStatusFile;

	static bool					cachingEnabled;
	static std::wstring			cacheFolderPath;
	static DataSize				defaultCachePageSize;
	static DataSize				defaultCacheCompletionThreshold;

	DataSize					cacheCompletionThreshold;

	static CacheFileNameMode	cacheFileNameMode;

protected:

	void						setDataCache						(DataCache *cache);
	DataCache				*	getDataCache						(void);

protected:

	bool						initializeCache						(const wchar_t *filePathFull, const wchar_t *filePathCache);

	DataCache				*	createDataCacheShared				(const wchar_t *filePathFull);
	static DataCache		*	getDataCacheShared					(const wchar_t *filePathFull);

	bool						createDataSources					(const wchar_t *filePathFull, DataCache *dataCache);
	void						closeDataSources					(void);

	void						setDataSourceFullFile				(DataSource *dataSource);
	void						setDataSourceCacheFile				(DataSource *dataSource);
	void						setDataSourceCacheStatusFile		(DataSource *dataSource);

	void						setCacheStatusFilePath				(const wchar_t *filePath);
	const wchar_t			*	getCacheStatusFilePath				(void);

	bool						setAutoCacheFilePathGUID			(const wchar_t *cacheFolder, PTRMI::GUID &guid);
	bool						setAutoCacheFilePathServerFilePath	(const wchar_t *cacheFolder, const wchar_t *serverPath);

public:

								DataSourceCache						(void);

	DataSourceForm				getDataSourceForm					(void);

	void						setCacheFilePath					(const wchar_t *filePathCache);
	const wchar_t			*	getCacheFilePath					(void);
	void						setCachePageSize					(DataSize pageSize);
	DataSize					getCachePageSize					(void);

	static DataSourcePtr		createNew							(const FilePath *filePathFull);

	void						destroy								(void);
	static DataSourceType		getDataSourceType					(void);

	bool						openForRead							(const FilePath *filepath, bool create = false);
	bool						openForWrite						(const FilePath *filepath, bool create = false);
	bool						openForReadWrite					(const FilePath *filepath, bool create = false);

	bool						validHandle							(void);
	static bool					isValidPath							(const FilePath *filepath);

	void						close								(void);
	bool						closeAndDelete						(void);

	Size						readBytes							(Data *buffer, Size number_bytes);
	Size						writeBytes							(const Data *buffer, Size number_bytes);

	Size						readBytesReadSet					(Data *buffer, DataSourceReadSet *readSet);

	DataSize					getFileSize							(void);

	bool						movePointerBy						(DataPointer numBytes);
	bool						movePointerTo						(DataPointer position);

	DataSource				*	getDataSourceFullFile				(void);
	DataSource				*	getDataSourceCacheFile				(void);
	DataSource				*	getDataSourceCacheStatusFile		(void);
	DataSource				*	getDataSourceForState				(DataCache::CachePageState state);

	static bool					setCacheFileNameMode				(CacheFileNameMode mode);
	static CacheFileNameMode	getCacheFileNameMode				(void);

	static bool					setCacheFolderPath					(const wchar_t *path);
	static const wchar_t	 *	getCacheFolderPath					(void);
	bool						setAutoCacheFilePath				(const wchar_t *serverFilePath, PTRMI::GUID &guid);

	bool						setCacheCompletionThreshold			(DataSize cacheCompletionThreshold);
	DataSize					getCacheCompletionThreshold			(void);

	static bool					setCachingEnabled					(bool enabled);
	static bool					getCachingEnabled					(void);

	static bool					setDefaultCachePageSize				(DataSize size);
	static DataSize				getDefaultCachePageSize				(void);

	static bool					setDefaultCacheCompletionThreshold	(DataSize cacheCompletionThreshold);
	static DataSize				getDefaultCacheCompletionThreshold	(void);

	static bool					getFullFileSubProtocol				(PTRMI::URL &fullFile, PTRMI::URL &subProtocol);
	static bool					getFullFileSubFilePath				(PTRMI::URL &fullFile, PTRMI::URL &fullFileSubProtocolFilePath);

	unsigned int				getBudgetParallelRead				(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed);

	bool						beginReadSet						(DataSourceReadSet *readSet);
	bool						endReadSet							(DataSourceReadSet *readSet);

	bool						isCached							(void) {return true;}
};





}
