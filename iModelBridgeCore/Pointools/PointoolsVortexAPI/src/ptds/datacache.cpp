#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataCache.h>
#include <ptds/DataSourceCache.h>
#include <ptds/DataCacheParallelRead.h>
#include <ptds/DataSourceReadSet.h>

// Pip Option
#include <iostream>


// ReadSet Handling for DataSourceCache and DataCache
// --------------------------------------------------
//
// Add original reads to DataSourceCache's readSet for use later
// getBudgetParallelRead creates a set of pages to be read pageSetOutOfCache (and also includes this in it's own costings)
// When all reads are done, from pagesOutOfCache, construct pageReadSet of contiguous page Reads
// Execute pageReadSet to a buffer and write pages to cache file on disk
// Execute DataSourceCache's readSet (knowing that all required pages should be in cache)
// Optimization: Cache budget system could construct the pageSetOutOfCache and also take it into consideration when doing it's own calculations





namespace ptds
{
																				// PTPODCFS (Pointools POD Cache File Status)
const DataCache::FileHeaderStr DataCache::DATA_CACHE_FILE_STATUS_HEADER_STR = 0x534346444F505450;

DataCache::DataCache(void)
{
	clear();
}


DataCache::~DataCache(void)
{
	destroyPageBuffer();
}


Status DataCache::clear(void)
{
    std::lock_guard<std::recursive_mutex> mutexScope(mutexCacheStats);
															// Flag that cache status file should be read
	setCacheStatusFileRead(false);
															// Initially no data
	setFileSize(DATA_CACHE_DEFAULT_FILE_SIZE);
	setNumCachePagesResident(DATA_CACHE_DEFAULT_NUM_PAGES_RESIDENT);

	setCachePageSize(DATA_CACHE_DEFAULT_CACHE_SIZE);

	clearCachePageResidencyStates();

	setPageBuffer(NULL);

	resetDirtyPagesWrittenSize();

	setCacheCompletionSizeThreshold(DATA_CACHE_DEFAULT_COMPLETION_SIZE_THRESHOLD);

	clearOutOfCachePages();

	return Status();
}


Status DataCache::open(const wchar_t *filePathFull, DataSourceCache *dataSourceCache)
{
	Status	status;

	if(filePathFull == NULL || getCacheFilePath() == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

															// Attempt to open cache
	if((status = openCache(filePathFull, getCacheFilePath(), dataSourceCache)).isFailed())
	{
															// Attempt to create the cache
		if((status = createCache(filePathFull, getCacheFilePath(), getCachePageSize(), dataSourceCache)).isFailed())
		{
			return status;
		}
	}
															// Create single page buffer used for windowed reads
	status = createPageBuffer(getCachePageSize());
															// Return status
	return status;
}


Status DataCache::close(DataSourceCache *dataSourceCache)
{
	return closeCache(dataSourceCache);
}


void DataCache::setNumCachePagesResident(CachePageIndex numPages)
{
	numCachePagesResident = numPages;
}


DataCache::CachePageIndex DataCache::getNumCachePagesResident(void)
{
	return numCachePagesResident;
}


DataCache::CachePageIndex DataCache::getNumCachePagesNonResident()
{
	return getNumCachePages() - getNumCachePagesResident();
}


DataCache::CachePageIndex DataCache::getNumCachePages(void)
{
	return cachePageResident.getNumBits();
}


void DataCache::setFileSize(DataSize size)
{
	fileSize = size;
}


DataCache::DataSize DataCache::getFileSize(void)
{
	return fileSize;
}


Status DataCache::readStartEndPages(DataPointer start, DataPointer end, Data *dest, DataSourceCache *dataSourceCache, bool &complete)
{
	CachePageIndex		pageStart, pageEnd;
	Status				status;
	DataPointer			pageStartOffset;
	Data			*	destEnd;

	if(dest == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

															// Default is read operation not completed
	complete	= false;
															// Get pages containing start and end pointers
	pageStart	= getCachePageIndex(start);
	pageEnd		= getCachePageIndex(end);
															// Get the index of the page to fetch
	if(pageStart == CachePageIndexNull || pageEnd == CachePageIndexNull || dest == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

															// Mutex single page read buffer
    std::lock_guard<std::recursive_mutex> mutexScope(mutexPageBuffer);
	
															// If Start and End are in same page, handle single windowed page
															// This is likely to be frequent, so it is handled explicitly
	if(pageStart == pageEnd)
	{
															// If last single page fetched isn't this page
		if(getPageBufferIndex() != pageStart)
		{
															// Fetch the page into a temporary buffer
			if((status = readPage(pageStart, getPageBuffer(), dataSourceCache)).isFailed())
				return status;
		}
															// Otherwise, use the existing buffer just read
		setPageBufferIndex(pageStart);
															// Get page offset (start of window) to read from
		pageStartOffset = getCachePageOffset(start);
															// Copy  fully windowed part of cache to destination
		memcpy(dest, &(getPageBuffer()[pageStartOffset]), (size_t)((end - start) + 1));

		complete = true;
															// Return OK
		return Status();
	}

															// Fetch the page into a temporary buffer
	if((status = readPage(pageStart, getPageBuffer(), dataSourceCache)).isFailed())
		return status;
															// Get position's offset within the page
	pageStartOffset = getCachePageOffset(start);
															// Copy first page with left window
	memcpy(dest, &(getPageBuffer()[pageStartOffset]), (size_t) (getCachePageSize() - pageStartOffset));


															// Fetch the page into a temporary buffer
	if((status = readPage(pageEnd, getPageBuffer(), dataSourceCache)).isFailed())
		return status;

	destEnd = dest + getCachePageDataPointer(pageEnd) - (getCachePageDataPointer(pageStart) + pageStartOffset);

															// Copy last page with right window
	memcpy(destEnd, getPageBuffer(), (size_t) (getCachePageOffset(end) + 1));

															// Return OK
	return Status();
}


Status DataCache::readBytes(DataPointer start, Data *dest, DataSize numBytes, DataSourceCache *dataSourceCache, DataSize &numBytesRead)
{
	CachePageIndex		startPage, endPage;
	CachePageIndex		numPagesRead;
	DataPointer			end;
	bool				complete;
	Status				status;


	numBytesRead = 0;

	if(dest == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);
															// Calculate pointer to last byte of data
	end = start + numBytes - 1;
															// Read start and end pages
	if((status = readStartEndPages(start, end, dest, dataSourceCache, complete)).isFailed() || complete)
	{
		numBytesRead = numBytes;
		return status;
	}
															// Get start and end pages indices
	startPage	= getCachePageIndex(start);
	endPage		= getCachePageIndex(end);
															// Read intermediate pages using grouped fetches
	status = readPageRange(startPage + 1, endPage - 1, dest + getCachePageRemainder(start), dataSourceCache, numPagesRead);

	if(status.isOK())
	{
		numBytesRead = numBytes;
	}
															// Return status
	return status;
}


Status DataCache::readPageRange(CachePageIndex startPage, CachePageIndex endPage, Data *dest, DataSourceCache *dataSourceCache, CachePageIndex &numPagesRead)
{
	CachePageIndex	numPages;
	CachePageIndex	page = startPage;
	Status			status;

	numPagesRead = 0;

	if(dest == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);
															// Process all non resident page ranges in main required range
	while(page != CachePageIndexNull && page <= endPage)
	{
															// Read as many pages with same state as possible, within the given main range
		if((status = readPageRangeContiguousState(page, endPage, dest, dataSourceCache, numPages)).isFailed())
			return status;
															// Advance destination memory pointer past read pages
		dest = dest + (numPages * getCachePageSize());
															// Advance current page index
		page += numPages;
															// Increment total num pages read counter
		numPagesRead += numPages;
	}
															// Return OK
	return Status();
}


bool DataCache::setCachePageResident(CachePageIndex cachePage, bool resident)
{
	return cachePageResident.setBit(cachePage, resident);
}


bool DataCache::getCachePageResident(CachePageIndex cachePage, bool &resident)
{
	return cachePageResident.getBit(cachePage, resident);
}


bool DataCache::isValidCachePageIndex(CachePageIndex cachePageIndex)
{
	return cachePageIndex < getNumCachePages();
}


Status DataCache::openCache(const wchar_t *filePathFull, const wchar_t *filePathCache, DataSourceCache *dataSourceCache)
{
	Status			status;
	DataSource *	dataSourceFullFile;
	DataSource *	dataSourceCacheFile;

	if(filePathFull == NULL || filePathCache == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Failed);

															// Get data source to full file
	if((dataSourceFullFile = dataSourceCache->getDataSourceFullFile()) == NULL)
		return Status(Status::Status_Error_Failed);
															// Get data source to access cache file	
	if((dataSourceCacheFile = dataSourceCache->getDataSourceCacheFile()) == NULL)
		return Status(Status::Status_Error_Failed);

															// Get cache status file's path
	std::wstring filePathCacheStatusStr;
	getDefaultCacheStatusFilePath(filePathCache, filePathCacheStatusStr);

															// If the cache status file hasn't been set up yet
	if(getCacheStatusFileRead() == false)
	{
															// Read cache status file
		if((status = readCacheStatusFile(filePathCacheStatusStr.c_str(), dataSourceCache)).isFailed())
		{
			return status;
		}
															// Remember that the cache status file has been read already
		setCacheStatusFileRead(true);
	}

															// Open full file for reading
    ptds::FilePath fullFilePath(filePathFull);
	if(dataSourceFullFile->openForRead(&fullFilePath) == false)
	{
		return Status(Status::Status_Error_File_Open_For_Read);
	}
															// Open main cache file for read/write
    ptds::FilePath cacheFilePath(filePathCache);
	if(dataSourceCacheFile->openForReadWrite(&cacheFilePath) == false)
	{
		dataSourceFullFile->close();
		return Status(Status::Status_Error_File_Open_For_Read_Write);
	}

															// Return OK
	return Status();
}


Status DataCache::createCache(const wchar_t *filePath, const wchar_t *cacheFilePath, DataSize initCachePageSize, DataSourceCache *dataSourceCache)
{
	Status status;

	if(filePath == NULL || cacheFilePath == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

	if(isValidCachePageSize(initCachePageSize) == false)
		return Status(Status::Status_Error_Bad_Parameter);

	ptds::FilePath	path(filePath);
	ptds::FilePath	cachePath(cacheFilePath);
	DataSize		fullFileSize;

	DataSource *	dataSourceFullFile;
	DataSource *	dataSourceCacheFile;

															// Get data sources required from owning DataSourceCache
	if((dataSourceFullFile = dataSourceCache->getDataSourceFullFile()) == NULL)
		return Status(Status::Status_Error_Failed);

	if((dataSourceCacheFile = dataSourceCache->getDataSourceCacheFile()) == NULL)
		return Status(Status::Status_Error_Failed);
															// Open file so the size can be obtained
	if(dataSourceFullFile->openForRead(&path) == false)
		return Status(Status::Status_Error_File_Open_For_Read);
															// Get size of full original file
	if((fullFileSize = dataSourceFullFile->getFileSize()) == 0)
		return Status(Status::Status_Error_File_Open_For_Read);


    std::lock_guard<std::recursive_mutex> mutexScope(mutexCacheStats);
															// Record the full file size
	setFileSize(fullFileSize);
															// Set the page size
	setCachePageSize(initCachePageSize);
															// Set the number of pages required and set up the residency bit array
	setNumCachePages(calculateNumPages(fullFileSize, getCachePageSize()));
															// Set all page residency states to non resident
	clearCachePageResidencyStates();

															// Create main cache file for read/write
	if(dataSourceCacheFile->openForReadWrite(&cachePath, true) == false)
		return Status(Status::Status_Error_Failed);

// Pip Option
std::cout << "Creating Cache..." << std::endl;
//dataSourceCacheFile->movePointerTo(fullFileSize - 1);
DataSize s;
DataSize b = 100 * 1024*1024;
unsigned char *buffer = new unsigned char[(size_t)b];

for(s = b; s <= fullFileSize; s += b)
{
std::cout << s << std::endl << std::flush;
	dataSourceCacheFile->writeBytes(buffer, b);
}

dataSourceCacheFile->writeBytes(buffer, fullFileSize - s - b);

dataSourceCacheFile->close();
delete []buffer;

if(dataSourceCacheFile->openForReadWrite(&cachePath, true) == false)
	return Status(Status::Status_Error_Failed);

std::cout << "Creating Cache Finished." << std::endl;


															// Get cache status file's path
	std::wstring cacheStatusPathStr;
	getDefaultCacheStatusFilePath(cacheFilePath, cacheStatusPathStr);

															// Read cache status file
	if((status = writeCacheStatusFile(cacheStatusPathStr.c_str(), dataSourceCache)).isFailed())
	{
		dataSourceFullFile->close();
		dataSourceCacheFile->close();
	}
															// Return status
	return status;
}


Status DataCache::closeCache(DataSourceCache *dataSourceCache)
{
	if(dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

	DataSource *dataSource;
	Status		status;
															// Make sure status file is up to date
	if((status = writeCacheStatusFile(getCacheStatusFilePath(), dataSourceCache)).isFailed())
		return status;
															// Close cache file
	if(dataSource = dataSourceCache->getDataSourceCacheFile())
		dataSource->close();

	return Status();
}


Status DataCache::readCacheStatusFile(const wchar_t *filePathCacheStatusFile, DataSourceCache *dataSourceCache)
{
	PTRMI::DataBuffer			buffer;
	FileHeaderStr				headerStr;
	CacheFileVersion			version;
	CachePageIndex				cachePagesResident;
	Status						status;
	DataSize					fullFileSize;
	DataSize					cachePageSize;

	if(filePathCacheStatusFile == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

    std::lock_guard<std::recursive_mutex> mutexScope(mutexStatusFile);

	buffer.setMode(PTRMI::DataBuffer::Mode_Internal);
															// Read file into buffer from data source
	if((status = buffer.readFileToBuffer(filePathCacheStatusFile, dataSourceCache->getDataSourceCacheStatusFile(), false)).isFailed())
		return status;
															// Get the file header string
	buffer >> headerStr;
	if(headerStr != DATA_CACHE_FILE_STATUS_HEADER_STR)
		return Status(Status::Status_Error_File_Type_Incorrect);
															// Get the file cache version number
	buffer >> version;
	if(version > getCacheFileCurrentVersion())
		return Status(Status::Status_Error_File_Newer_Version);


    std::lock_guard<std::recursive_mutex> mutexScopeCacheStats(mutexCacheStats);
															// Get the size of the full original file
	buffer >> fullFileSize;
	setFileSize(fullFileSize);
															// Get the cache page size
	buffer >> cachePageSize;
	setCachePageSize(cachePageSize);
															// Get number of pages resident in the cache
	buffer >> cachePagesResident;
	setNumCachePagesResident(cachePagesResident);
															// Set number of caches pages and initialize
	setNumCachePages(calculateNumPages(fullFileSize, cachePageSize));
															// Read page residency bit array
	if(cachePageResident.readFromBuffer(buffer) == false)
		return Status(Status::Status_Error_File_Read);
															// Return OK
	return Status();
}


Status DataCache::writeCacheStatusFile(const wchar_t *filePathCacheStatusFile, DataSourceCache *dataSourceCache)
{
	PTRMI::DataBuffer			buffer;
	PTRMI::DataBuffer::DataSize	bufferSize;
	Status						status;
	ptds::FilePath				path(filePathCacheStatusFile);

	if(filePathCacheStatusFile == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

    std::lock_guard<std::recursive_mutex> mutexScopeStatsFile(mutexStatusFile);

    std::lock_guard<std::recursive_mutex> mutexScopeCacheStats(mutexCacheStats);
															// Reserve a conservatively sized buffer
	bufferSize = cachePageResident.getStorageSizeBytes() + 1024;
															// Create buffer
	if((status = buffer.createInternalBuffer(bufferSize)).isFailed())
		return status;
															// Write Header string
	buffer << DATA_CACHE_FILE_STATUS_HEADER_STR;
															// Write file version number
	buffer << getCacheFileCurrentVersion();
															// Write size of full original file
	buffer << getFileSize();
															// Write cache page size
	buffer << getCachePageSize();
															// Write number of cache pages resident
	buffer << getNumCachePagesResident();
															// Write bit array to buffer
	cachePageResident.writeToBuffer(buffer);
															// Write buffer to data source
	if((status = buffer.writeFileFromBuffer(filePathCacheStatusFile, dataSourceCache->getDataSourceCacheStatusFile())).isFailed())
		return status;

	return status;
}


void DataCache::getDefaultCacheStatusFilePath(const wchar_t * cacheFilePath, std::wstring &cacheInfoPathStr)
{
	if(cacheFilePath)
	{
		cacheInfoPathStr = cacheFilePath;

		cacheInfoPathStr.append(DATA_CACHE_STATUS_FILE_PATH_ENDING);
	}

}


DataCache::CacheFileVersion DataCache::getCacheFileCurrentVersion(void)
{
	return DATA_CACHE_FILE_CURRENT_VERSION;
}


void DataCache::setCachePageSize(DataSize size)
{
	cachePageSize = size;
}


DataCache::DataSize DataCache::getCachePageSize(void)
{
	return cachePageSize;
}


bool DataCache::isValidCachePageSize(DataSize size)
{
	return (size > 0 && size < DATA_CACHE_MAX_RECOMMENDED_PAGE_SIZE);
}


DataCache::CachePageIndex DataCache::calculateNumPages(DataSize fullFileSize, DataSize pageSize)
{
	if(fullFileSize == 0 || pageSize == 0)
		return 0;

	CachePageIndex	pages = static_cast<CachePageIndex>(fullFileSize / pageSize);

	if((fullFileSize & pages) > 0)
		++pages;

	return pages;
}


void DataCache::setNumCachePages(CachePageIndex pages)
{
	cachePageResident.initialize(pages);
}


DataCache::CachePageIndex DataCache::getCachePageIndex(DataPointer position)
{
	if(getCachePageSize() == 0)
		return 0;

    return static_cast<CachePageIndex>(position / getCachePageSize());
}


DataSource::DataPointer DataCache::getCachePageOffset(DataPointer position)
{
	if(getCachePageSize() > 0)
	{
		return position % getCachePageSize();
	}

	return 0;
}	


DataCache::DataPointer DataCache::getCachePageRemainder(DataPointer position)
{
															// Return remaining size (including given position)
	return getCachePageSize() - getCachePageOffset(position);
}


ptds::DataCache::CachePageState DataCache::getPageState(CachePageIndex page)
{
	bool value;

	if(cachePageResident.getBit(page, value))
	{
		if(value)
		{
			return PageStateResident;
		}
		else
		{
			return PageStateNonResident;
		}
	}

	return PageStateNULL;
}


Status DataCache::readPage(CachePageIndex page, Data *dest, DataSourceCache *dataSourceCache)
{
	if(page == CachePageIndexNull || dest == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

	Status			status;
	CachePageState	state;

	state = getPageState(page);
															// Read page from cache or full original file
	if((status = readPageFromDataSource(page, dest, dataSourceCache->getDataSourceForState(state))).isFailed())
		return status;
															// If source was full file, write page to cache
	if(state == PageStateNonResident)
	{
		status = writeCachePage(dest, page, dataSourceCache);
	}

	return status;
}


Status DataCache::readPageFromDataSource(CachePageIndex page, Data *dest, DataSource *dataSource)
{
	if(page != CachePageIndexNull && dest && dataSource)
	{
															// Get data pointer to start of page
		DataPointer startPointer = getCachePageDataPointer(page);
															// Move access to start of page
		if(dataSource->movePointerTo(startPointer) == false)
		{
			return Status(Status::Status_Error_File_Move_Pointer);
		}

		DataSize read;
															// Read a single page
		if((read = dataSource->readBytes(dest, getCachePageSize())) != getCachePageSize())
		{
															// If not the last page, this is an error condition
			if(isLastPage(page) == false)
			{
				return Status(Status::Status_Error_File_Read);
			}
															// If last page, make sure amount read is equal to remaining data in last page
			if(read != getLastPageDataSize())
			{
				return Status(Status::Status_Error_File_Read);
			}
		}
	}
	else
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	return Status();
}


Status DataCache::readPageRangeFromDataSource(CachePageIndex pageStart, CachePageIndex pageEnd, Data *dest, DataSource *dataSource, CachePageIndex &numPagesRead)
{
	CachePageIndex	numPages;
	DataSize		dataSize = 0;
	Status			status;

	numPagesRead = 0;

	if(isValidCachePageIndex(pageStart) == false || isValidCachePageIndex(pageEnd) == false || dest == NULL || dataSource == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}
															// Read single page separately
	if(pageStart == pageEnd)
	{
		if((status = readPageFromDataSource(pageStart, dest, dataSource)).isFailed())
			return status;

		numPagesRead = 1;

		return status;
	}

	if(isLastPage(pageEnd))
	{
															// Account for last page read separately
		dataSize = getLastPageDataSize();

		numPagesRead = 1;
															// Discount last page from range read
		--pageEnd;
	}
															// Make sure the range is valid
	if(pageEnd < pageStart)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

															// Calculate number of pages to be read
	numPages = pageEnd - pageStart + 1;
	dataSize += numPages * getCachePageSize();
															// Move read pointer to start of first page
	dataSource->movePointerTo(getCachePageDataPointer(pageStart));
															// Read required number of contiguous pages
	if(dataSource->readBytes(dest, dataSize) == dataSize)
	{
															// Return full number of pages read
		numPagesRead += numPages;
															// Return read OK
		return Status();
	}
															// Return no pages read
	numPagesRead = 0;
															// Return failed
	return Status(Status::Status_Error_File_Read);
}


Status DataCache::createPageBuffer(DataSize size)
{
    std::lock_guard<std::recursive_mutex> mutexScope(mutexPageBuffer);

	if(size == 0)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

															// If an existing buffer is smaller, reallocate a new larger buffer
	if(pageBuffer == NULL || (pageBuffer && getCachePageSize() < size))
	{
		destroyPageBuffer();

		if((pageBuffer = new Data[(size_t)size]) == NULL)
			return Status(Status::Status_Error_Memory_Allocation);

		setPageBufferIndex(CachePageIndexNull);
	}
															// Return OK
	return Status();
}


void DataCache::setPageBuffer(Data *buffer)
{
	pageBuffer = buffer;
}


DataCache::Data *DataCache::getPageBuffer(void)
{
	return pageBuffer;
}

void DataCache::destroyPageBuffer(void)
{
    std::lock_guard<std::recursive_mutex> mutexScope(mutexPageBuffer);

	if(getPageBuffer() != NULL)
	{
		delete []getPageBuffer();

		setPageBuffer(NULL);
	}
}


ptds::DataCache::DataPointer DataCache::getCachePageDataPointer(CachePageIndex page)
{
	if(page != CachePageIndexNull)
	{
		return page * getCachePageSize();
	}

	return 0;
}


Status DataCache::readPageRangeContiguousState(CachePageIndex pageStart, CachePageIndex pageMax, Data *dest, DataSourceCache *dataSourceCache, CachePageIndex &numPagesRead)
{
	CachePageIndex		pageRangeEnd;
	CachePageState		state;
	DataSource		*	dataSource;
	Status				status;

	if(dest == NULL || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);


	if(isValidCachePageIndex(pageStart) == false || isValidCachePageIndex(pageMax) == false || dest == NULL)
		return Status(Status::Status_Error_Bad_Parameter);
															// Get size of contiguous range with same residency state as the first page
	if((status = getPageRangeContiguousState(pageStart, pageMax, pageRangeEnd, state)).isFailed())
		return status;
															// Get the data source required to read the contiguous state
	if((dataSource = dataSourceCache->getDataSourceForState(state)) == NULL)
		return Status(Status::Status_Error_Failed);
															// Read contiguous state range from data source	
	if((status = readPageRangeFromDataSource(pageStart, pageRangeEnd, dest, dataSource, numPagesRead)).isFailed())
		return status;
															// If range was non resident
	if(state == PageStateNonResident)
	{
 		if((status = writeCachePageRange(dest, pageStart, pageRangeEnd, dataSourceCache)).isFailed())
 			return status;
	}
															// Return OK
	return Status();
}


DataSource::DataSize DataCache::getCachePageRangeDataSize(CachePageIndex pageStart, CachePageIndex pageEnd)
{
	DataSize	rangeDataSize = 0;

															// If range has only one page
	if(pageStart == pageEnd)
	{
															// If single page is last page of full file
		if(isLastPage(pageEnd))
		{
															// Return last page data size
			return getLastPageDataSize();
		}
															// Return size of single cache page
		return getCachePageSize();
	}
	else
	if(pageStart < pageEnd)
	{
															// If proper range and end is last page
		if(isLastPage(pageEnd))
		{
															// Include data in last page and discount one whole page
			rangeDataSize = getLastPageDataSize();
			--pageEnd;
		}
															// Add a range of whole pages
		rangeDataSize += (pageEnd - pageStart + 1) * getCachePageSize();
	}
															// Return range size
	return rangeDataSize;
}


Status DataCache::writeCachePage(Data *source, CachePageIndex page, DataSourceCache *dataSourceCache)
{
	return writeCachePageRange(source, page, page, dataSourceCache);
}


Status DataCache::writeCachePageRange(Data *source, CachePageIndex pageStart, CachePageIndex pageEnd, DataSourceCache *dataSourceCache)
{

	if(source == NULL || isValidCachePageIndex(pageStart) == false || isValidCachePageIndex(pageEnd) == false || dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

	if(pageEnd < pageStart)
		return Status(Status::Status_Error_Bad_Parameter);

	CachePageIndex		numPages;
	DataSize			dataSize;
	DataPointer			destPointer;
	DataSource		*	dataSource;
	Status				status;

	if((dataSource = dataSourceCache->getDataSourceCacheFile()) == NULL)
		return Status(Status::Status_Error_Failed);

	numPages = pageEnd - pageStart + 1;
															// Get size of start-end data block of pages
	dataSize = getCachePageRangeDataSize(pageStart, pageEnd);
															// Get write position in file
	destPointer = getCachePageDataPointer(pageStart);
															// Move to write position
	if(dataSource->movePointerTo(destPointer) == false)
		return Status(Status::Status_Error_File_Move_Pointer);
															// Write number of pages to file

// Pip Option
// extern pt::SimpleTimer test_timer[];
// test_timer[6].start();

	if(dataSource->writeBytes(source, dataSize) != dataSize)
		return Status(Status::Status_Error_File_Write);
// Pip Option
// test_timer[6].stop();


    std::lock_guard<std::recursive_mutex> mutexScope(mutexCacheStats);
															// Set residency bits to resident for all written pages
	setPageRangeResidencyState(pageStart, pageEnd, PageStateResident);
															// Update number of cache pages resident in cache
	setNumCachePagesResident(getNumCachePagesResident() + numPages);
															// Update metrics of amount of data written since last
	setDirtyPagesWrittenSize(getDirtyPagesWrittenSize() + dataSize);

															// Update cache status file if requirements are met
	manageCacheStatusFileUpdate(dataSourceCache);

															// Return OK
	return status;
}


void DataCache::setDirtyPagesWrittenSize(DataSize size)
{
	dirtyPagesWrittenSize = size;
}


void DataCache::resetDirtyPagesWrittenSize(void)
{
	dirtyPagesWrittenSize = DATA_CACHE_DEFAULT_DATA_SIZE_DIRTY_PAGES_WRITTEN;
}


DataCache::DataSize DataCache::getDirtyPagesWrittenSize(void)
{
	return dirtyPagesWrittenSize;
}


void DataCache::incrementDirtyPagesWrittenSize(DataSize size)
{
	setDirtyPagesWrittenSize(getDirtyPagesWrittenSize() + size);
}


Status DataCache::manageCacheStatusFileUpdate(DataSourceCache *dataSourceCache, bool closing)
{
	Status	status;

    std::lock_guard<std::recursive_mutex> mutexScope(mutexCacheStats);

// Pip Option
//outputStatusToLog();
															// If enough data has been written to the cache file
															// or a user is closing and there is unwritten data
	if(getDirtyPagesWrittenSize() >= getDirtyPagesWrittenSizeThreshold() || (closing && getDirtyPagesWrittenSize() > 0))
	{
															// Rewrite the cache status file to reflect changes
		if((status = writeCacheStatusFile(getCacheStatusFilePath(), dataSourceCache)).isFailed())
		{
			return status;
		}
															// Set dity data counter to zero
		resetDirtyPagesWrittenSize();
	}

	return status;
}


Status DataCache::setPageRangeResidencyState(CachePageIndex pageStart, CachePageIndex pageEnd, CachePageState state)
{
	if(isValidCachePageIndex(pageStart) == false || isValidCachePageIndex(pageEnd) == false)
		return Status(Status::Status_Error_Bad_Parameter);

	CachePageIndex	page;
	bool			resident = (state == PageStateResident);

	for(page = pageStart; page <= pageEnd; page++)
	{
		setCachePageResident(page, resident);
	}
															// Return OK
	return Status();
}


Status DataCache::clearCachePageResidencyStates(void)
{
    std::lock_guard<std::recursive_mutex> mutexScope(mutexCacheStats);

	cachePageResident.clearArray();

	setNumCachePagesResident(0);

	return Status();
}


Status DataCache::getPageRangeContiguousState(CachePageIndex pageStart, CachePageIndex pageMax, CachePageIndex &pageRangeEnd, CachePageState &state)
{
	if(isValidCachePageIndex(pageStart) == false || isValidCachePageIndex(pageMax) == false)
		return Status(Status::Status_Error_Bad_Parameter);

	CachePageIndex	page;
															// Get initial state
	state = getPageState(pageStart);

	for(page = pageStart + 1; page <= pageMax; page++)
	{
		if(getPageState(page) != state)
		{
															// Return last page as end of contiguous range
			pageRangeEnd = page - 1;

			return Status();
		}
	}
															// Return whole range is of type
	pageRangeEnd = pageMax;
															// Return OK
	return Status();
}


void DataCache::setPageBufferIndex(CachePageIndex page)
{
	pageBufferIndex = page;
}


DataCache::CachePageIndex DataCache::getPageBufferIndex(void)
{
	return pageBufferIndex;
}


DataCache::DataSize DataCache::getDirtyPagesWrittenSizeThreshold(void)
{
	return DATA_CACHE_DEFAULT_DIRTY_PAGES_WRITTEN_THRESHOLD;
}


void DataCache::setCacheStatusFilePath(const wchar_t *filePath)
{
	if(filePath)
	{
		filePathCacheStatus = filePath;
	}
}


const wchar_t *DataCache::getCacheStatusFilePath(void)
{
	return filePathCacheStatus.c_str();
}


Status DataCache::getCacheStats(DataSize &bytesFull, DataSize &bytesEmpty)
{
    std::lock_guard<std::recursive_mutex> mutexScope(mutexCacheStats);

	bytesFull	= getNumCachePagesResident() * getCachePageSize();
	bytesEmpty	= getFileSize() - bytesFull;

	return Status();
}


bool DataCache::isCacheComplete(void)
{
    std::lock_guard<std::recursive_mutex> mutexScope(mutexCacheStats);

	return (getNumCachePagesResident() == getNumCachePages());
}


bool DataCache::isCacheCompletable(void)
{
															// If cache is already complete, it is not completable
	if(isCacheComplete())
		return false;
															// If completion feature disabled, return false
	if(getCacheCompletionSizeThreshold() == 0)
		return false;
															// Calculate data size of number of non resident pages
	DataSize sizeNonResident = getNumCachePagesNonResident() * getCachePageSize();
															// If size of non resident pages is in threshold, return true
	return (sizeNonResident <= getCacheCompletionSizeThreshold());
}


Status DataCache::completeCache(DataSourceCache *dataSourceCache)
{
	CachePageIndex		page;
	CachePageIndex		numPages;
	Status				status;


    std::lock_guard<std::recursive_mutex> mutexScopeCacheStats(mutexCacheStats);
    std::lock_guard<std::recursive_mutex> mutexScopeStatusFile(mutexStatusFile);

															// If no cache pages, no file to create
	if((numPages = getNumCachePages()) == 0)
		return Status(Status::Status_Error_Failed);
															// Fetch non resident pages one by one
															// (Note: Replace this with a range optimized version)
	for(page = 0; page < numPages; page ++)
	{
		if(getPageState(page) == PageStateNonResident)
		{
			this->readPage(page, getPageBuffer(), dataSourceCache);
		}
	}
															// Make sure there are no outstanding caches
	if(getNumCachePagesResident() != getNumCachePages())
	{
		return Status(Status::Status_Error_Failed);
	}
															// Write final status file
	status = writeCacheStatusFile(getCacheStatusFilePath(), dataSourceCache);

															// Return status
	return status;
}


void DataCache::setCacheFilePath(const wchar_t *filePath)
{
	filePathCache = filePath;
}


const wchar_t * DataCache::getCacheFilePath(void)
{
	return filePathCache.c_str();
}


bool DataCache::isLastPage(CachePageIndex page)
{
	return (page + 1) == getNumCachePages();
}


DataSource::DataSize DataCache::getLastPageDataSize(void)
{
															// Calculate amount of data in the last page (may not use whole page)
	return getFileSize() - ((getNumCachePages() - 1) * getCachePageSize());
}


void DataCache::setCacheCompletionSizeThreshold(DataSize size)
{
	cacheCompletionSizeThreshold = size;
}


DataSource::DataSize DataCache::getCacheCompletionSizeThreshold(void)
{
	return cacheCompletionSizeThreshold;
}


void DataCache::setCacheStatusFileRead(bool read)
{
	cacheStatusFileRead = read;
}


bool DataCache::getCacheStatusFileRead(void)
{
	return cacheStatusFileRead;
}


void DataCache::outputStatusToLog(void)
{
	wchar_t	message[2048];

	swprintf(message, 2048, L"Cache Pages: %d %d %d", getNumCachePages(), getNumCachePagesResident(), getPercentageCachePagesResident());

	Status::log(message, L"");
}


unsigned int DataCache::getPercentageCachePagesResident(void)
{
	double	pagesResident	= getNumCachePagesResident();
	double	pagesTotal		= getNumCachePages();

	if(pagesTotal > 0)
	{
		return static_cast<unsigned int>(100.0 * pagesResident / pagesTotal);
	}

	return 0;
}


unsigned int DataCache::getBudgetParallelRead(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed)
{
	ParallelReadPriorityQueue					queue;
	unsigned int								numReads;
	bool										budgetExhausted = false;
	unsigned int								t;
	DataSourceRead							*	dataSourceRead;
	std::vector<DataCacheParallelRead>			completeRead;
	DataSize									cost;
	unsigned int								numPointsRead;
	CachePageIndex								page;

	budgetUsed = 0;
															// Make sure reads exist
	if((numReads = readSet.getNumReads()) == 0)
		return 0;
															// For all reads in read set
	for(t = 0; t < numReads; t++)
	{
		if(dataSourceRead = readSet.getRead(t))
		{
			DataCacheParallelRead	parallelRead;
															// Convert read to parallel read info
			parallelRead.setReadSize(dataSourceRead->getSize());
			parallelRead.setCurrentReadPosition(dataSourceRead->getPosition());
			parallelRead.setItemSize(dataSourceRead->getItemSize());
															// Insert read info into min priority queue
			queue.push(parallelRead);
		}
	}
															// Calculate minimum budget for this voxel
	if(budget > 0)
	{
															// This is one cache page per 
		DataSize minimumBudget = readSet.getNumReads() * getCachePageSize();
															// Assign minimum budget to budget to guarantee streamed points (not from cache)
		budget = std::max(budget, minimumBudget);
	}

															// While there is still processing to do and budget is not exhausted
	while(queue.empty() == false && budgetExhausted == false)
	{
															// Get read with least number of points so far
		DataCacheParallelRead parallelRead = queue.top();
															// Pop top item
		queue.pop();
															// Get the cost of making the cache read
		cost = getPageReadCost(parallelRead, page);
															// If read fits in given budget
		if(budgetUsed + cost <= budget)
		{
															// If page is out of cache, add to unique page set
			if(cost > 0)
			{
				addOutOfCachePage(page);
			}
															// Accept the read and advance the parallel read one cache page
			consumeRead(queue, parallelRead);
															// Add the cost to the budget
			budgetUsed += cost;
															// If this parallel read is not complete
			if(parallelRead.isReadComplete() == false)
			{
															// Insert parallel read back into priority queue
				queue.push(parallelRead);
			}
			else
			{
															// Otherwise, this parallel read is complete
				completeRead.push_back(parallelRead);
			}
		}
		else
		{
															// Push last (unprocessed) read to be processed
			completeRead.push_back(parallelRead);
															// Last read is out of budget, so terminate
			budgetExhausted = true;
		}
	}

															// Get minimum numPoints of reads in priority queue (first is smallest)
	if(queue.empty() == false)
	{
		numPointsRead = queue.top().getCurrentNumPointsRead();
	}
	else
	if(completeRead.empty() == false)
	{
															// Otherwise, get the first value from the completed reads
		numPointsRead = completeRead[0].getCurrentNumPointsRead();
	}
	else
	{
		return false;
	}
															// Get the smallest numPoints in the parallel reads
															// because this is what this read can  provide
	while(completeRead.empty() == false)
	{
		numPointsRead = std::min(numPointsRead, completeRead.back().getCurrentNumPointsRead());
		completeRead.pop_back();
	}
															// Return number of points read
	return numPointsRead;
}


ptds::DataCache::DataSize DataCache::getPageReadCost(DataCacheParallelRead &parallelRead, CachePageIndex &pageIndex)
{
	bool resident;

	pageIndex = getCachePageIndex(parallelRead.getCurrentReadPosition());
															// Query if cache page is resident
	if(getCachePageResident(pageIndex, resident))
	{
															// If page not already scheduled to be read
															// and is not resident in the cache
		if(getOutOfCachePage(pageIndex) == false && resident == false)
		{
															// return read cost as being the size of a cache page
			return getCachePageSize();
		}
															// Resident or already scheduled pages have zero cost
		return 0;
	}
															// Cache page doesn't exist, so return failure
	return 0;
}


bool DataCache::consumeRead(ParallelReadPriorityQueue &queue, DataCacheParallelRead &parallelRead)
{
															// Get remaining cache page that has been read
															// At start this may be partial, but after will be whole cache pages
	DataSize		dataSizeRead;
															// Useful data read is that remaining in cache page
	dataSizeRead = getCachePageRemainder(parallelRead.getCurrentReadPosition());
															// Add remaining cache page data
	parallelRead.setCurrentReadSize(parallelRead.getCurrentReadSize() + dataSizeRead);
															// If enough has been read to fulfill full request
	if(parallelRead.isReadComplete())
	{
															// Clip current read size to the full read size
		parallelRead.setCurrentReadSize(parallelRead.getReadSize());
	}
															// Move read position to next cache page
	parallelRead.setCurrentReadPosition(parallelRead.getCurrentReadPosition() + dataSizeRead);
															// Number of points read is total data read divided by bytes required to represent item
    parallelRead.setCurrentNumPointsRead(static_cast<uint>(parallelRead.getCurrentReadSize() / parallelRead.getItemSize()));

															// Return OK
	return true;
}


void DataCache::addOutOfCachePage(CachePageIndex page)
{
															// Add page to out of cache set
	outOfCachePageSet.insert(page);
}


bool DataCache::getOutOfCachePage(CachePageIndex page)
{
															// Return true if page already exists in set
	return outOfCachePageSet.find(page) != outOfCachePageSet.end();
}


void DataCache::clearOutOfCachePages(void)
{
	outOfCachePageSet.clear();
}


bool DataCache::beginReadSet(void)
{
	clearOutOfCachePages();

	return true;
}


bool DataCache::endReadSet(void)
{
	return true;
}


DataSize DataCache::getOutOfCachePageReadSet(CachePageIndexSet pageSet, DataSourceReadSet &readSet)
{

	CachePageIndexSet::iterator	start, end, i;
	CachePageIndex				firstPage, lastPage;
	DataSize					readSize;
	DataSize					totalReadSize = 0;

	start	= pageSet.begin();
	end		= pageSet.end();

															// For all items in page set
	for(i = start; i != end; )
	{
		firstPage = *i;

		do
		{
			lastPage = *i;
			i++;

		} while(i != end && *i == lastPage + 1);

															// Get size of this read
		readSize = (lastPage - firstPage + 1) * getCachePageSize();
															// Add page read to read set
		readSet.addRead(DataSourceRead(0, 0, getCachePageDataPointer(firstPage), readSize, NULL));
															// Count total read size
		totalReadSize += readSize;
	}

															// Return read size added by this call
	return totalReadSize;
}


/*
	if(dataSourceCache == NULL)
		return Status(Status::Status_Error_Bad_Parameter);
															// Get cache's full file data source
	if((dataSourceFullFile = dataSourceCache->getDataSourceFullFile()) == NULL)
		return Status(Status::Status_Error_Cache_Full_File_Data_Source_Not_Found);
															// Get cache's cache file data source
	if((dataSourceCacheFile = dataSourceCache->getDataSourceCacheFile()) == NULL)
		return Status(Status::Status_Error_Cache_File_Data_Source_Not_Found);
*/

Status DataCache::readOutOfCachePageReadSet(DataSource &dataSourceFullFile, DataSourceReadSet &readSet, PTRMI::DataBuffer &dataBuffer)
{
															// If there are no out of cache pages, return OK
	if(outOfCachePageSet.size() == 0)
		return Status();

	DataSize			readSize;
	Data			*	buffer;
	Status				status;
															// If no data to be read, exit
	if((readSize = readSet.getTotalReadSize()) == 0)
		return Status(Status::Status_Error_Cache_Read_Set_Failed);
															// Allocate memory from the DataBuffer
	dataBuffer.createInternalBuffer(static_cast<PTRMI::DataBuffer::DataSize>(readSize));
	if((buffer = dataBuffer.allocate(static_cast<PTRMI::DataBuffer::DataSize>(readSize))) == NULL)
		return Status(Status::Status_Error_Memory_Allocation);

															// Execute the read set from the full file
	if(dataSourceFullFile.readBytesReadSet(buffer, &readSet) != readSize)
		return Status(Status::Status_Error_Cache_Read_Set_Failed);

															// Return OK
	return Status();
}


Status DataCache::writeOutOfCachePageReadSet(DataSourceCache &dataSourceCache, DataSourceReadSet &readSet, PTRMI::DataBuffer &dataBuffer)
{

	DataSourceReadSet::ReadIndex		numReads, t;
	DataSourceRead					*	read;
	DataSize							readSize;
	CachePageIndex						numCachePages;
	CachePageIndex						pageStart, pageEnd;
	Data							*	buffer;
	Status								status;

															// Validate parameters
	if(dataBuffer.getBuffer() == NULL)
		return Status(Status::Status_Error_Bad_Parameter);

															// Get total number of reads in read set
	if((numReads = readSet.getNumReads()) == 0)
		return Status();
															// Get source buffer
	buffer = dataBuffer.getBuffer();
															// For each read in read set
	for(t = 0; t < numReads; t++)
	{
		if((read = readSet.getRead(t)) == NULL)
			return Status(Status::Status_Error_Cache_Read_Set_Failed);
															// Get read size (should be multiple of cache page size)
		if((readSize = read->getSize()) == 0)
			return Status(Status::Status_Error_Cache_Read_Set_Failed);
															// Get exact number of pages represented in this read
		if((numCachePages = getNumCachePagesInDataSize(readSize, false)) == 0)
			return Status(Status::Status_Error_Cache_Read_Set_Failed);

		pageStart	= getCachePageIndex(read->getPosition());
		pageEnd		= pageStart + numCachePages - 1;

		if((status = writeCachePageRange(buffer, pageStart, pageEnd, &dataSourceCache)).isFailed())
			return status;
															// Advance source buffer to next read
		buffer += readSize;
	}

															// Return OK
	return Status();
}


DataCache::CachePageIndex DataCache::getNumCachePagesInDataSize(DataSize dataSize, bool allowRemainder)
{
															// Make sure cache page size is defined
	if(getCachePageSize() == 0)
		return 0;

	CachePageIndex numPages;
															// Get number of pages that could fit in given data size
	numPages = static_cast<CachePageIndex>(dataSize / getCachePageSize());
															// If there is a remainder
	if(dataSize % getCachePageSize() > 0)
	{
															// If there should be no remainder, return zero as an error
		if(allowRemainder == false)
			return 0;
															// Otherwise, data size requires another page
		++numPages;
	}
															// Return number of pages
	return numPages;
}


DataSize DataCache::readOutOfCachePages(DataSourceCache &dataSourceCache)
{
	DataSourceReadSet	readSet;
	PTRMI::DataBuffer	dataBuffer;
	DataSize			readSize;
	Status				status;
															// Make sure full file data source exists
	if(dataSourceCache.getDataSourceFullFile() == NULL)
		return 0;

// Pip Option
// extern pt::SimpleTimer test_timer[];
// test_timer[3].start();
															// Construct a read set for all required out of cache pages
															// This is based on the budget parallel read system

	if((readSize = getOutOfCachePageReadSet(outOfCachePageSet, readSet)) == 0)
		return 0;

// test_timer[3].stop();
// 
// test_timer[4].start();
															// Read the read set of out of cache pages
	if(readOutOfCachePageReadSet(*dataSourceCache.getDataSourceFullFile(), readSet, dataBuffer).isFailed())
		return 0;

// test_timer[4].stop();
// 
// test_timer[5].start();
															// Write out of cache pages to cache file
	if(writeOutOfCachePageReadSet(dataSourceCache, readSet, dataBuffer).isFailed())
		return 0;

// test_timer[5].stop();
															// Return total amount of data read
	return readSize;
}


DataSize DataCache::readBytesReadSet(DataSourceCache &dataSourceCache, Data *buffer, DataSourceReadSet &readSet)
{
	// fetch all out of cache pages and write them to cache
	// Run read set on cache file, with guarantee that all pages are in the file (maybe validate this)

	DataSize		cagePageReadSize;
	DataSize		readSize;
	DataSourcePtr	dataSourceCacheFile;

															// Get size of main data read set
	if((readSize = readSet.getTotalReadSize()) == 0)
		return 0;
															// Make sure cache file data source exists
	if((dataSourceCacheFile = dataSourceCache.getDataSourceCacheFile()) == NULL)
		return 0;

// Pip Option
// extern pt::SimpleTimer test_timer[];
// test_timer[1].start();
															// Make sure all required cache pages are read into the cache file
	cagePageReadSize = readOutOfCachePages(dataSourceCache);

// test_timer[1].stop();


// test_timer[2].start();

	if(readSet.executeReadSet(*dataSourceCache.getDataSourceCacheFile(), buffer) == NULL)
		return 0;

// test_timer[2].stop();
															// Return the total data read
	return readSize;
}

} // End ptds namespace
