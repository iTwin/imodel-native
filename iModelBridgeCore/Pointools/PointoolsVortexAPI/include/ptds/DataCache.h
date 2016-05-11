#pragma once

#include <PTRMI/Status.h>
#include <ptds/DataSource.h>
#include <ptds/BitArray.h>

#define DATA_CACHE_DEFAULT_FILE_SIZE						0
#define DATA_CACHE_DEFAULT_NUM_PAGES_RESIDENT				0
#define DATA_CACHE_DEFAULT_CACHE_SIZE						(1024 * 128)		// Default cache page size 128k
#define DATA_CACHE_DEFAULT_DATA_SIZE_DIRTY_PAGES_WRITTEN	0
#define DATA_CACHE_DEFAULT_DIRTY_PAGES_WRITTEN_THRESHOLD	(1024 * 1024 * 50)	// Update status for every MB written to the cache file
#define DATA_CACHE_DEFAULT_COMPLETION_SIZE_THRESHOLD		(1024 * 1024 * 5)	// Default automatic completion size threshold

#define DATA_CACHE_MAX_RECOMMENDED_PAGE_SIZE				(1024 * 1024 * 10)	// 10MB page size max recommended

#define DATA_CACHE_STATUS_FILE_HEADER_STRING				L"PTPODTMPCS"
#define DATA_CACHE_STATUS_FILE_PATH_ENDING					L"s"


namespace ptds
{

typedef PTRMI::Status	Status;

class DataSourceCache;
class DataSourceReadSet;
class DataCacheParallelRead;


#define DATA_CACHE_FILE_CURRENT_VERSION		0x0001;


class DataCache
{
public:

	typedef unsigned char				Data;
	typedef DataSource::DataSize		DataSize;
	typedef unsigned long				CachePageIndex;
	typedef DataSource::DataPointer		DataPointer;

protected:

	typedef BitArray<>					PageResidentBitArray;
	typedef unsigned short				CacheFileVersion;
	typedef uint64_t			FileHeaderStr;

	typedef std::priority_queue<DataCacheParallelRead>	ParallelReadPriorityQueue;

	typedef std::set<CachePageIndex>	CachePageIndexSet;

public:

	const static CachePageIndex		CachePageIndexNull = ~(static_cast<CachePageIndex>(0));

	enum CachePageState
	{
		PageStateNULL,

		PageStateNonResident,
		PageStateResident
	};

protected:

	bool					cacheStatusFileRead;

	DataSize				fileSize;
	DataSize				cachePageSize;
	DataSize				cacheCompletionSizeThreshold;

	std::wstring			filePathCache;
	std::wstring			filePathCacheStatus;

	PageResidentBitArray	cachePageResident;
	CachePageIndex			numCachePagesResident;	// Item may not be thread safe
	DataSize				dirtyPagesWrittenSize;	// Item may not be thread safe

	Data				*	pageBuffer;
	CachePageIndex			pageBufferIndex;

	CachePageIndexSet		outOfCachePageSet;

    std::recursive_mutex    mutexCacheStats;
    std::recursive_mutex	mutexPageBuffer;
    std::recursive_mutex	mutexStatusFile;

protected:

	static const FileHeaderStr DATA_CACHE_FILE_STATUS_HEADER_STR;


protected:

	Status					createPageBuffer					(DataSize size);
	void					setPageBuffer						(Data *buffer);
	Data				*	getPageBuffer						(void);
	void					destroyPageBuffer					(void);

	void					setCacheStatusFileRead				(bool read);
	bool					getCacheStatusFileRead				(void);

	void					setFileSize							(DataSize size);
	DataSize				getFileSize							(void);

	DataSize				getCachePageRangeDataSize			(CachePageIndex start, CachePageIndex end);
	DataSize				getLastPageDataSize					(void);

	bool					isValidCachePageSize				(DataSize size);
	CachePageIndex			calculateNumPages					(DataSize fullFileSize, DataSize pageSize);

	void					setNumCachePages					(CachePageIndex pages);

	bool					isLastPage							(CachePageIndex page);

	void					setNumCachePagesResident			(CachePageIndex numPages);

	Status					clearCachePageResidencyStates		(void);

	bool					isValidCachePageIndex				(CachePageIndex cachePageIndex);

	CachePageState			getPageState						(CachePageIndex page);
	DataPointer				getCachePageDataPointer				(CachePageIndex page);

	CachePageIndex			getCachePageIndex					(DataPointer position);
	DataPointer				getCachePageOffset					(DataPointer position);
	DataPointer				getCachePageRemainder				(DataPointer position);
	CachePageIndex			getNumCachePagesInDataSize			(DataSize readSize, bool allowRemainder);
	Status					getPageRangeContiguousState			(CachePageIndex pageStart, CachePageIndex pageMax, CachePageIndex &pageRangeEnd, CachePageState &state);

	Status					readStartEndPages					(DataPointer start, DataPointer end, Data *dest, DataSourceCache *dataSourceCache, bool &complete);

	Status					readPage							(CachePageIndex page, Data *dest, DataSourceCache *dataSourceCache);
	Status					readPageFromDataSource				(CachePageIndex page, Data *dest, DataSource *dataSource);

	void					setPageBufferIndex					(CachePageIndex page);
	CachePageIndex			getPageBufferIndex					(void);

	Status					readPageRange						(CachePageIndex pageStart, CachePageIndex pageEnd, Data *dest, DataSourceCache *dataSourceCache, CachePageIndex &numPagesRead);
	Status					readPageRangeContiguousState		(CachePageIndex pageStart, CachePageIndex pageMax, Data *dest, DataSourceCache *dataSourceCache, CachePageIndex &numPagesRead);
	Status					readPageRangeFromDataSource			(CachePageIndex pageStart, CachePageIndex pageEnd, Data *dest, DataSource *dataSource, CachePageIndex &numPagesRead);
	Status					writeCachePage						(Data *source, CachePageIndex page, DataSourceCache *dataSourceCache);
	Status					writeCachePageRange					(Data *source, CachePageIndex pageStart, CachePageIndex pageEnd, DataSourceCache *dataSourceCache);

	Status					openCache							(const wchar_t *filePathFull, const wchar_t *filePathCache, DataSourceCache *dataSourceCache);
	Status					createCache							(const wchar_t *filePathFull, const wchar_t *filePathCache, DataSize initCachePageSize, DataSourceCache *dataSourceCache);
	Status					closeCache							(DataSourceCache *dataSourceCache);

	Status					setPageRangeResidencyState			(CachePageIndex pageStart, CachePageIndex pageEnd, CachePageState state);
	bool					setCachePageResident				(CachePageIndex cachePage, bool resident);
	bool					getCachePageResident				(CachePageIndex cachePage, bool &resident);

	Status					readCacheStatusFile					(const wchar_t *filePathCacheStatusFile, DataSourceCache *dataSourceCache);
	Status					writeCacheStatusFile				(const wchar_t *filePathCacheStatusFile, DataSourceCache *dataSourceCache);

	void					setDirtyPagesWrittenSize			(DataSize size);
	DataCache::DataSize		getDirtyPagesWrittenSize			(void);
	void					resetDirtyPagesWrittenSize			(void);
	void					incrementDirtyPagesWrittenSize		(DataSize size);

	DataSize				getDirtyPagesWrittenSizeThreshold	(void);

	DataSize				getPageReadCost						(DataCacheParallelRead & readInfo, CachePageIndex &pageIndex);
	bool					consumeRead							(ParallelReadPriorityQueue &queue, DataCacheParallelRead &readInfo);

	void					addOutOfCachePage					(CachePageIndex page);
	bool					getOutOfCachePage					(CachePageIndex page);
	void					clearOutOfCachePages				(void);
	DataSize				getOutOfCachePageReadSet			(CachePageIndexSet pageSet, DataSourceReadSet &readSet);
	Status					readOutOfCachePageReadSet			(DataSource &dataSourceFullFile, DataSourceReadSet &readSet, PTRMI::DataBuffer &dataBuffer);
	Status					writeOutOfCachePageReadSet			(DataSourceCache &dataSourceCache, DataSourceReadSet &readSet, PTRMI::DataBuffer &dataBuffer);
	DataSize				readOutOfCachePages					(DataSourceCache &dataSourceCache);

public:

							DataCache							(void);
						   ~DataCache							(void);

	Status					clear								(void);

	Status					open								(const wchar_t *filePath, DataSourceCache *dataSourceCache);
	Status					close								(DataSourceCache *dataSourceCache);

	void					setCacheCompletionSizeThreshold		(DataSize size);
	DataSize				getCacheCompletionSizeThreshold		(void);

	static void				getDefaultCacheStatusFilePath		(const wchar_t * cacheFilePath, std::wstring &cacheStatusPathStr);

	void					setCacheFilePath					(const wchar_t *filePath);
	const wchar_t		*	getCacheFilePath					(void);

	void					setCacheStatusFilePath				(const wchar_t *filePath);
	const wchar_t		*	getCacheStatusFilePath				(void);

	Status					readBytes							(ptds::DataSource::DataPointer start, Data *dest, DataSize numBytes, DataSourceCache *dataSourceCache, DataSize &numBytesRead);
	DataSize				readBytesReadSet					(DataSourceCache &dataSourceCache, Data *buffer, DataSourceReadSet &readSet);

	Status					getCacheStats						(DataSize &bytesFull, DataSize &bytesEmpty);

	bool					isCacheComplete						(void);
	bool					isCacheCompletable					(void);
	Status					completeCache						(DataSourceCache *dataSourceCache);

	CacheFileVersion		getCacheFileCurrentVersion			(void);

	void					setCachePageSize					(DataSize size);
	DataSize				getCachePageSize					(void);

	Status					manageCacheStatusFileUpdate			(DataSourceCache *dataSourceCache, bool closing = false);

	CachePageIndex			getNumCachePages					(void);
	CachePageIndex			getNumCachePagesResident			(void);
	CachePageIndex			getNumCachePagesNonResident			(void);
	unsigned int			getPercentageCachePagesResident		(void);

	void					outputStatusToLog					(void);

	unsigned int			getBudgetParallelRead				(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed);

	bool					beginReadSet						(void);
	bool					endReadSet							(void);

};





} // End ptds namespace