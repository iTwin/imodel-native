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
    m_file.SetBogus();
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

	if(BeFileStatus::Success == m_file.Open(path, BeFileAccess::Read))
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

	if(BeFileStatus::Success == m_file.Create(path, /*createAlways*/true))
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

	BeFileStatus status;
    if (create)
        status = m_file.Create(path, /*createAlways*/true);
    else
        status = m_file.Open(path, BeFileAccess::ReadWrite);

	if(BeFileStatus::Success == status)
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
    return BeFileStatus::Success == m_file.SetPointer(numBytes, BeFileSeekOrigin::Current);
}


bool DataSourceFile::movePointerTo(DataPointer numBytes)
{
	Super::movePointerTo(numBytes);

    return BeFileStatus::Success == m_file.SetPointer(numBytes, BeFileSeekOrigin::Begin);		
}


bool DataSourceFile::validHandle(void)
{
	return m_file.IsOpen();
}

void DataSourceFile::close(void)
{
	if(validHandle())
	{
        m_file.Close();

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
    
    return BeFileNameStatus::Success == BeFileName::BeDeleteFile(filePath.path());
}

DataSource::Size DataSourceFile::readBytes(Data *buffer, Size numBytes)
{
	uint32_t numRead = 0;

	if(getReadSetEnabled() && isReadSetDefined())
	{
		return readBytesFrom(buffer, getDataPointer(), numBytes);
	}

	if(validHandle())
	{
		beginRead(numBytes);

		if(BeFileStatus::Success != m_file.Read(buffer, &numRead, (uint32_t)numBytes))
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
    uint32_t written = 0;
    m_file.Write(&written, buffer, (uint32_t)number_bytes);

	return written;
}

int64_t DataSourceFile::getFileSize(void) 
{ 
    uint64_t size = 0;
    m_file.GetSize(size);

	return size;
};


bool DataSourceFile::isValidPath(const FilePath *filepath)
{
	if(filepath == NULL)
		return false;

	if(filepath->path())
	{
		std::wstring p = filepath->path();

		unsigned int i = static_cast<uint>(p.find(':'));
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
		url.setProtocol(PTRMI::URL(PTRMI::URL::PT_PTFL));
		url += filePath->path();
	}
	else
	{
		url = L"";
	}
}



} // End ptds namespace
