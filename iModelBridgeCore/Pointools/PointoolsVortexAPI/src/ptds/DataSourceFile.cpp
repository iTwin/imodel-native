#include "PointoolsVortexAPIInternal.h"
#include <iostream>

#include <ptds/DataSourceFile.h>

#include <PTRMI/Name.h>

#ifndef NO_DATA_SOURCE_SERVER
#include <PTRMI/Status.h>
#endif

namespace ptds
{

DataSourceFile::DataSourceFile(void)
{
	invalidateHandle();
}


DataSourceFile::~DataSourceFile(void)
{
															// Make sure file is closed
	close();
}


bool DataSourceFile::openForRead(const FilePath *filepath, bool create)
{
	if(filepath == NULL)
	{
		return false;
	}

	wchar_t path[PT_MAXPATH];
	filepath->fullpath(path);

	setOpenState(DataSourceStateClosed);

	handle = CreateFileW(path,
		GENERIC_READ,				 // open for writing
		FILE_SHARE_READ,			 // do not share
		NULL,						 // no security
		OPEN_EXISTING,				 // open existing only
		FILE_ATTRIBUTE_NORMAL,		 // normal file
		NULL);						 // optimization hints

	if(validHandle())
	{
		filePath = (*filepath);
		PTRMI::Status::log(L"Open for read : ", path);
		canRead = true;

		setOpenState(DataSourceStateOpenForRead);

		return true;
	}

	PTRMI::Status::log(L"Error: Open for read failed", path);

	return false;
}

bool DataSourceFile::openForWrite(const FilePath *filepath, bool create)
{
	if(filepath == NULL)
		return false;

	wchar_t path[PT_MAXPATH];
	filepath->fullpath(path);

	setOpenState(DataSourceStateClosed);

	handle = CreateFileW(path,
		GENERIC_WRITE,				// open for writing
		NULL,						// do not share
		NULL,						// no security
		CREATE_ALWAYS,				// overwrite existing
		FILE_ATTRIBUTE_NORMAL |		// normal file
		FILE_FLAG_WRITE_THROUGH,	// optimization hints
		NULL);

	if(validHandle())
	{
		filePath = (*filepath);
		canWrite = true;

		setOpenState(DataSourceStateOpenForWrite);

		PTRMI::Status::log(L"Open for write : ", path);

		return true;
	}

	PTRMI::Status::log(L"Error: Open for write failed", path);

	return false;
}

bool DataSourceFile::openForReadWrite(const FilePath *filepath, bool create)
{
	if(filepath == NULL)
		return false;

	wchar_t path[PT_MAXPATH];
	filepath->fullpath(path);


	setOpenState(DataSourceStateClosed);

	DWORD openExistingOrCreate = OPEN_EXISTING;
												// If Create specified, create if file doesn't exist
	if(create)
	{
		openExistingOrCreate = OPEN_ALWAYS;
	}

	handle = CreateFileW(path,
		GENERIC_READ | GENERIC_WRITE,			// Open for Read and Write
		FILE_SHARE_READ | FILE_SHARE_WRITE,		// Share reads and writes
		NULL,									// no security
		openExistingOrCreate,					// open existing or create a new file
		FILE_ATTRIBUTE_NORMAL |					// normal file
		FILE_FLAG_WRITE_THROUGH,
		NULL);									// optimization hints

	if(validHandle())
	{
		filePath = (*filepath);
		canWrite = true;
		canRead = true;

		setOpenState(DataSourceStateOpenForReadWrite);

		PTRMI::Status::log(L"Open for read write : ", path);

		return true;
	}

	PTRMI::Status::log(L"Error: Open for read write failed", path);

	return false;
}

bool DataSourceFile::movePointerBy(DataPointer numBytes)
{
	LARGE_INTEGER li;

	li.QuadPart = numBytes;

	long highPart = li.HighPart;
	long lowPart = li.LowPart;

	DWORD lo = SetFilePointer(handle, lowPart, &highPart, FILE_CURRENT);

	return (lowPart == li.LowPart && highPart == li.HighPart) ? true : false;
}


bool DataSourceFile::movePointerTo(DataPointer numBytes)
{
	LARGE_INTEGER li;

	Super::movePointerTo(numBytes);

	li.QuadPart = numBytes;

	long highPart = li.HighPart;
	long lowPart = li.LowPart;

	DWORD lo = SetFilePointer(handle, lowPart, &highPart, FILE_BEGIN);

	return (lo == li.LowPart && highPart == li.HighPart) ? true : false;			
}


bool DataSourceFile::validHandle(void)
{
	return handle != INVALID_HANDLE_VALUE;
}


void DataSourceFile::invalidateHandle(void)
{
	handle = INVALID_HANDLE_VALUE;
}


void DataSourceFile::close(void)
{
	if(validHandle())
	{
		CloseHandle(handle);
		invalidateHandle();

		setOpenState(DataSourceStateClosed);

		const FilePath *filePath;
		if(filePath = getFilePath())
		{
			PTRMI::Status::log(L"Closed file : ", filePath->path());
		}
	}
}

bool DataSourceFile::closeAndDelete(void)
{
	close();

	return DeleteFileW(filePath.path()) != 0;
}

DataSource::Size DataSourceFile::readBytes(Data *buffer, Size numBytes)
{
	DWORD numRead;

	if(getReadSetEnabled() && isReadSetDefined())
	{
		return readBytesFrom(buffer, getDataPointer(), numBytes);
	}

	if(handle)
	{
		beginRead(numBytes);

		if(ReadFile(handle, buffer, numBytes, &numRead, NULL) == FALSE)
		{
			return 0;
		}

		endRead(numRead);
	}

	return numRead;
}


DataSource::Size DataSourceFile::readBytesFrom(Data *buffer, DataPointer position, Size numBytes)
{

#ifndef NO_DATA_SOURCE_SERVER

															// If a read set is defined
	if(getReadSetEnabled() && isReadSetDefined())
	{
														// Add item to the read set
		ptds::DataSize numRead = addReadSetItem(buffer, getDataPointer(), numBytes);
														// Advance local read pointer
		setDataPointer(position + numRead);

		return numRead;
	}
	else
#endif
	{
		if(movePointerTo(position) == false)
			return 0;

		return readBytes(buffer, numBytes);
	}
}


DataSource::Size DataSourceFile::writeBytes(const Data *buffer, Size number_bytes)
{
	DWORD W; 
	
	WriteFile(handle, buffer, static_cast<DWORD>(number_bytes), &W, 0);

	return static_cast<Size>(W);
}

int64_t DataSourceFile::getFileSize(void) 
{ 
	LARGE_INTEGER li;
	DWORD s;
	li.LowPart = GetFileSize(handle, &s); 
	li.HighPart = (LONG)s;
	return li.QuadPart;
};


bool DataSourceFile::isValidPath(const FilePath *filepath)
{
	if(filepath == NULL)
		return false;

	if(filepath->path())
	{
		std::wstring p = filepath->path();

		unsigned int i = p.find(':');
															// If not present or drive letter, return is file path
		if(i == -1 || i == 1)
			return true;
	}
															// Return not file path
	return false;
}

DataSourcePtr DataSourceFile::createNew(const FilePath *path)
{
	if(path == NULL)
		return NULL;

	DataSourcePtr dataSource;

	if(isValidPath(path) == false)
		return NULL;

	if((dataSource = new DataSourceFile()) == NULL)
		return NULL;

	return dataSource;
}

DataSource::DataSourceType DataSourceFile::getDataSourceType( void )
{
	return DataSourceTypeFile;
}


void DataSourceFile::destroy(void)
{
	delete this;
}


DataSource::DataSourceForm DataSourceFile::getDataSourceForm(void)
{
	return DataSourceFormLocal;
}


void DataSourceFile::getHostName(PTRMI::Name &hostName)
{
	PTRMI::URL		url;
	PTRMI::URL		fileHostURL;
															// Clear the given Name
	hostName.clear();
															// Get file path URL
	getURL(url);
															// Extract the file host string (drive letter if absolute or .\ if relative)
	url.getProtocolHostAddress(fileHostURL);
															// Set URL part of Name
	hostName.set(fileHostURL.getString());
}


void DataSourceFile::getURL(PTRMI::URL &url)
{
	const FilePath *filePath;

	if(filePath = getFilePath())
	{
		url.setProtocol(PTRMI::URL::URL(PTRMI::URL::PT_PTFL));
		url += filePath->path();
	}
	else
	{
		url = L"";
	}
}



} // End ptds namespace
