#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataSource.h>
#include <ptds/DataSourceNull.h>
#include <ptds/DataSourceFile.h>
#include <ptds/DataSourceStructuredStorage.h>
#include <ptds/DataSourceReadSet.h>
#include <ptds/DataSourceStringIO.h>
#include <PTRMI/URL.h>
#include <ptds/DataSourceMultiReadSet.h>

#define DATA_SOURCE_DEFAULT_STREAM_BUDGET	(30 * 1024 * 1024)


namespace ptds
{


DataSource::DataSource(void)
{
															// Initially closed
	setOpenState(DataSourceStateClosed);
															// Pointer initially at zero
	resetDataPointer();

//	analyzer.initialize();
															// Initially no batch read set
	setReadSetClientID(NULL);
	setReadSetEnabled(false);
	setReadSet(NULL);
	canRead = false;
	canWrite = false;
}


DataSource::DataSourceType DataSource::getPathDataSourceType(const FilePath *path)
{
															// If path is a standard file path, return type
	if(DataSourceFile::isValidPath(path))
		return DataSourceFile::getDataSourceType();
															// If path is a structured storage (external), return type
	if(DataSourceStructuredStorage::isValidPath(path))
		return DataSourceStructuredStorage::getDataSourceType();
															// Path type not recognized so return NULL type
	return DataSourceNull::getDataSourceType();
}


void DataSource::setFilePath(const FilePath *path)
{
	if(path)
	{
		filePath = (*path);
	}
}

const FilePath *DataSource::getFilePath(void)
{
	return &filePath;
}


void DataSource::getURL(PTRMI::URL &url)
{
	if(getFilePath())
	{
		url.set(getFilePath()->path());
	}
}


void DataSource::setOpenState(DataSourceOpenState state)
{
	openState = state;
}


DataSource::DataSourceOpenState DataSource::getOpenState(void)
{
	return openState;
}

void DataSource::setDataPointer(DataPointer pointer)
{
	dataPointer = pointer;
}


DataSource::DataPointer DataSource::getDataPointer(void)
{
	return dataPointer;
}


void DataSource::resetDataPointer(void)
{
	setDataPointer(DATA_SOURCE_DEFAULT_DATA_POINTER);
}


void DataSource::advanceDataPointer(DataSize numBytes)
{
	setDataPointer(getDataPointer() + numBytes);
}


bool DataSource::moveFile(const FilePath *path, const FilePath *newPath)
{
	return false;
}

#ifndef NO_DATA_SOURCE_SERVER

DataSource::Size DataSource::addReadSetItem(Data *buffer, DataPointer position, Size numBytes)
{
															// Add a read set item based on the 
	if(isReadSetDefined())
	{
		if(getReadSet()->addRead(DataSourceRead(getReadSetClientID(), 0, position, numBytes, buffer)))
		{
			return numBytes;
		}
	}

	return 0;
}

#endif

bool DataSource::movePointerBy(DataPointer numBytes)
{
	setDataPointer(getDataPointer() + numBytes);

	return true;
}


bool DataSource::movePointerTo(DataPointer position)
{
	setDataPointer(position);

	return true;
}


#ifndef NO_DATA_SOURCE_SERVER

unsigned int DataSource::getBudgetParallelRead(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed)
{
	return getBudgetParallelReadNormal(budget, readSet, budgetUsed);
}


unsigned int DataSource::getBudgetParallelReadNormal(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed)
{
	DataSourceReadSet::ItemSize totalItemSize;
	DataSize					totalReadSize;
	unsigned int				numPoints = 0;
															// Divide budget by total channel requirement for one point
	if(totalItemSize = readSet.getTotalItemSize())
	{
		totalReadSize = readSet.getTotalReadSize();
															// If budget is less than required for full read
		if(budget <= totalReadSize)
		{
															// Number of points read is the number of whole points that can be read using the budget
			numPoints		= static_cast<unsigned int>(budget / totalItemSize);
		}
		else
		{
															// Budget is greater than necessary, so clip num points to specified read size
			numPoints		= static_cast<unsigned int>(totalReadSize / totalItemSize);
		}

															// Budget not used is the remainder
		budgetUsed = numPoints * totalItemSize;
	}
															// Item size not defined, so return zero
	return numPoints;
}

#endif


void DataSource::beginRead(DataSize numBytes)
{

}


void DataSource::endRead(DataSize numBytes)
{

}


void DataSource::setReadSet(DataSourceReadSet *initReadSet)
{
	if(getReadSetEnabled())
	{
		readSet = initReadSet;
	}
	else
	{
		readSet = NULL;
	}
}


DataSourceReadSet *DataSource::getReadSet(void)
{
	return readSet;
}


bool DataSource::beginReadSet(DataSourceReadSet *readSet)
{
	if(readSet == NULL)
		return false;

	setReadSet(readSet);

	return true;
}


bool DataSource::endReadSet(DataSourceReadSet *readSet)
{
	if(readSet == NULL || getReadSet() != readSet)
		return false;

	setReadSet(NULL);

	return true;
}


void DataSource::clearReadSet(void)
{
	setReadSet(NULL);
}

#ifndef NO_DATA_SOURCE_SERVER

DataSource::Size DataSource::readBytesReadSet(Data *buffer, DataSourceReadSet *readSet)
{
	if(buffer == NULL || readSet == NULL)
		return 0;
															// Default behaviour is to read locally from data source
	if(readSet->executeReadSet(*this, buffer) == NULL)
		return 0;
															// Return amount read
	return readSet->getTotalReadSize();
}


DataSource::Size DataSource::readBytesMultiReadSet(Data *buffer, DataSourceMultiReadSet *multiReadSet)
{
	DataSource::Size	result = 0;

	if(buffer == NULL || multiReadSet == NULL)
	{
		return 0;
	}
															// Default behaviour is to read locally from data source
	if(multiReadSet->executeMultiReadSet(*this, buffer))
	{
															// Get result
		result = multiReadSet->getTotalReadSize();
	}
															// Delete all dynamically allocated data that's no longer needed
	multiReadSet->clear(true);
															// Return amount read
	return result;
}

#endif

#ifndef NO_DATA_SOURCE_SERVER

ptds::DataSize DataSource::getReadSetTotalSize(void)
{
	if(isReadSetDefined())
	{
		return getReadSet()->getTotalReadSize();
	}

	return 0;
}

#endif

void DataSource::beginReadSetClientID(void *clientID)
{
	setReadSetClientID(clientID);
}


void DataSource::endReadSetClientID(void)
{
	setReadSetClientID(NULL);
}


void DataSource::setReadSetClientID(void *id)
{
	readSetClientID = id;
}


void *DataSource::getReadSetClientID(void)
{
	return readSetClientID;
}


void DataSource::setReadSetEnabled(bool enabled)
{
	readSetEnabled = enabled;

	setReadSet(NULL);
}


bool DataSource::getReadSetEnabled(void)
{
	return readSetEnabled;
}

DataSource::Size DataSource::writeString( const pt::String &str )
{
	return DataSourceStringIO::write( this, str );
}

DataSource::Size DataSource::readString( class pt::String &str )
{
	return DataSourceStringIO::read( this, str );
}


bool DataSource::isReadWrite(void) const
{
	return canRead && canWrite;
}


bool DataSource::isReadOnly(void) const
{
	return canRead && !canWrite;
}

void DataSource::getHostName(PTRMI::Name &hostName)
{
															// Default to 'not supported'
	hostName.clear();
}





} // End ptds namespace