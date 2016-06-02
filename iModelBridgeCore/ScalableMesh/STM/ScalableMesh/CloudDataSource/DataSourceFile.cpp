#include "stdafx.h"
#include "DataSourceFile.h"
#include "DataSourceAccount.h"


DataSourceFile::DataSourceFile(DataSourceAccount *sourceAccount) : DataSource(sourceAccount)
{

}

DataSourceFile::~DataSourceFile(void)
{

}


DataSourceStatus DataSourceFile::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
	DataSourceURL				url;
	std::wstring				filePath;
	std::ios_base::open_mode	streamMode = std::ios_base::binary;
	DataSourceStatus			status;

	status = Super::open(sourceURL, sourceMode);
	if (status.isFailed())
		return status;

	if(getAccount() == nullptr)
		return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);

	getURL(url);
	
	url.getFilePath(filePath);

	switch (sourceMode)
	{
	case DataSourceMode_Write:
		streamMode |= std::ios_base::out;
		break;

	case DataSourceMode_Read:
	default:
		streamMode |= std::ios_base::in;
		break;
	}

	stream.open(filePath, streamMode);

	if (stream.is_open())
	{
		return DataSourceStatus();
	}

	return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
}

DataSourceStatus DataSourceFile::close(void)
{
	stream.close();

	return DataSourceStatus();
}

DataSource::DataSize DataSourceFile::getSize(void)
{
	DataPtr		originalPtr;
	DataSize	length;
															// Take a copy of the current file pointer so it can be restored
	originalPtr = getStream().tellg();
															// Seek to end of file
	try
	{
		getStream().seekg(0, getStream().end);
	}
	catch (...)
	{
		DataSourceStatus(Status_Error_Seek);

		return 0;
	}
															// Get file ptr at end of file
	length = getStream().tellg();
															// Restore original file ptr
	std::streampos pos = originalPtr;
	getStream().seekg(pos);

															// Return the length of the file
	return length;
}

/*
DataSourceStatus DataSourceFile::read(Buffer * dest, DataSize destSize)
{
	DataSize	size;

	if ((size = getSize()) > destSize)
		return DataSourceStatus(DataSourceStatus::Status_Error_Dest_Buffer_Too_Small);

	getStream().read(reinterpret_cast<char *>(dest), size);

	if(getStream())
		return DataSourceStatus();

	return DataSourceStatus(DataSourceStatus::Status_Error_Read);
}
*/

DataSourceStatus DataSourceFile::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size)
{
	if (size > destSize)
		return DataSourceStatus(DataSourceStatus::Status_Error_Dest_Buffer_Too_Small);

	if (size > 0)
	{
		getStream().read(reinterpret_cast<char *>(dest), size);
	}
	else
	{
		std::streampos originalPosition = getStream().tellg();

		getStream().seekg(0, getStream().end);
		DataSize fileSize = getStream().tellg();
		getStream().seekg(originalPosition);

		DataSize sizeToRead = fileSize - originalPosition;

		if(sizeToRead > destSize)
			return DataSourceStatus(DataSourceStatus::Status_Error_Dest_Buffer_Too_Small);

		getStream().read(reinterpret_cast<char *>(dest), sizeToRead);
	}

	readSize = getStream().gcount();

	if(size > 0 && readSize != size)
		return DataSourceStatus(DataSourceStatus::Status_Error_EOF);

	if (getStream())
	{
		return DataSourceStatus();
	}

	return DataSourceStatus(DataSourceStatus::Status_Error_EOF);
}

DataSourceStatus DataSourceFile::write(Buffer * source, DataSize size)
{
	getStream().write(reinterpret_cast<char *>(source), size);

	if (getStream())
	{
		return DataSourceStatus();
	}

	return DataSourceStatus(DataSourceStatus::Status_Error_Write);
}

DataSourceStatus DataSourceFile::move(DataPtr position)
{
	std::streampos pos = position;

	try
	{
		getStream().seekg(pos);
	}
	catch (...)
	{
		return DataSourceStatus(DataSourceStatus::Status_Error_Seek);
	}

	return DataSourceStatus();
}
