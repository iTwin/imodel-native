#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataSourceMemory.h>


namespace ptds
{

DataSourceMemory::DataSourceMemory(void)
{

}

DataSourceMemory::~DataSourceMemory(void)
{
	close();
}


DataSource::DataSourceForm DataSourceMemory::getDataSourceForm(void)
{
	return DataSource::DataSourceFormLocal;
}


DataSourcePtr DataSourceMemory::createNew(const FilePath *path, DataSource::Data *sourceBuffer, DataSource::DataSize sourceBufferSize)
{
	if(path == NULL || (sourceBuffer && sourceBufferSize == 0))
		return NULL;

	DataSourceMemory *dataSource;

	if(isValidPath(path) == false)
		return NULL;

	if((dataSource = new DataSourceMemory()) == NULL)
		return NULL;

	if(sourceBuffer)
	{
		dataSource->dataBuffer.setExternalBuffer(sourceBuffer, static_cast<PTRMI::DataBuffer::DataSize>(sourceBufferSize));
		dataSource->dataBuffer.setMode(PTRMI::DataBuffer::Mode_External);
		dataSource->dataBuffer.setWritePtr(static_cast<PTRMI::DataBuffer::DataSize>(sourceBufferSize));
	}
	else
	{
		dataSource->dataBuffer.setMode(PTRMI::DataBuffer::Mode_Internal);
	}

	
	return dataSource;
}


void DataSourceMemory::destroy(void)
{
	delete this;
}


bool DataSourceMemory::openForRead(const FilePath *filepath, bool create)
{
	dataBuffer.setReadPtr(0);

	setOpenState(DataSourceStateOpenForRead);

	return true;
}


bool DataSourceMemory::openForWrite(const FilePath *filepath, bool create)
{
	dataBuffer.setReadPtr(0);
	dataBuffer.setWritePtr(0);

	setOpenState(DataSourceStateOpenForWrite);

	return true;
}


bool DataSourceMemory::openForReadWrite(const FilePath *filepath, bool create /*= false*/)
{
	dataBuffer.setReadPtr(0);
	dataBuffer.setWritePtr(0);

	setOpenState(DataSourceStateOpenForReadWrite);

	return true;
}


bool DataSourceMemory::validHandle(void)
{
	return true;
}


bool DataSourceMemory::isValidPath(const FilePath *filepath)
{
	if(filepath == NULL)
		return false;

	PTRMI::URL url = filepath->path();

	if(url.isValidURL() == false)
		return false;

	PTRMI::URL	protocol;

	if(url.getProtocol(protocol) == false)
		return false;

	return (protocol == PTRMI::URL::PT_PTMY);
}


void DataSourceMemory::close(void)
{
	setOpenState(DataSourceStateClosed);
}


bool DataSourceMemory::closeAndDelete(void)
{
	close();

	return true;
}


DataSource::Size DataSourceMemory::readBytes(Data *buffer, Size numBytes)
{
	return dataBuffer.readFromBuffer(buffer, static_cast<PTRMI::DataBuffer::DataSize>(numBytes));
}


DataSource::Size DataSourceMemory::writeBytes(const Data *buffer, Size numBytes)
{
	dataBuffer.writeToBuffer(buffer, static_cast<PTRMI::DataBuffer::DataSize>(numBytes));

	return 0;
}


DataSource::Size DataSourceMemory::readBytesFrom(Data *buffer, DataPointer position, Size numBytes)
{
	if((dataBuffer.setReadPtr(static_cast<PTRMI::DataBuffer::DataSize>(position))).isFailed())
	{
		return 0;
	}

	return readBytes(buffer, numBytes);
}


DataSource::DataSize DataSourceMemory::getFileSize(void)
{
	return dataBuffer.getBufferSize();
}


bool DataSourceMemory::movePointerBy(DataPointer numBytes)
{
	DataPointer position;

	if(getOpenState() == DataSource::DataSourceStateOpenForRead || DataSource::DataSourceStateOpenForReadWrite)
	{
		position = dataBuffer.getReadPtr();
	}
	else
	{
		position = dataBuffer.getWritePtr();
	}

	return movePointerTo(position);
}


bool DataSourceMemory::movePointerTo(DataPointer numBytes)
{
															// If mode is Write or ReadWrite, move Write pointer first so that extra space is created if not already present
	if(getOpenState() == DataSource::DataSourceStateOpenForWrite || getOpenState() == DataSourceStateOpenForReadWrite)
	{
		if(dataBuffer.setWritePtr(static_cast<PTRMI::DataBuffer::DataSize>(numBytes)).isFailed())
		{
			dataBuffer.allocateTo(static_cast<PTRMI::DataBuffer::DataSize>(numBytes));

			if(dataBuffer.setWritePtr(static_cast<PTRMI::DataBuffer::DataSize>(numBytes)).isFailed())
			{
				return false;
			}
		}
	}
															// If mode is Read or ReadWrite, move read pointer
	if(getOpenState() == DataSourceStateOpenForRead || getOpenState() == DataSourceStateOpenForReadWrite)
	{
		if(dataBuffer.setReadPtr(static_cast<PTRMI::DataBuffer::DataSize>(numBytes)).isFailed())
		{
			return false;
		}
	}
															// Return OK
	return true;
}


DataSource::Data *DataSourceMemory::getBuffer(DataSize &bufferSize)
{
	bufferSize = dataBuffer.getDataSize();

	return dataBuffer.getBuffer();
}


} // End ptds namespace

