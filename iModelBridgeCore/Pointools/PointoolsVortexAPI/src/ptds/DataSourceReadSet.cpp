#include "PointoolsVortexAPIInternal.h"
#include <ptds/DataSourceReadSet.h>

#include <ptcloud2/voxel.h>

namespace ptds
{

DataSourceReadSet::DataSourceReadSet(void)
{
	clear();
}


void DataSourceReadSet::clear(void)
{
	readSet.clear();

	setTotalItemSize(0);

	setTotalReadSize(0);

}


bool DataSourceReadSet::addRead(DataSourceRead const&read)
{
	readSet.push_back(read);
															// Accumulate total item size
	setTotalItemSize(getTotalItemSize() + read.getItemSize());
															// Accumulate total read size
	setTotalReadSize(getTotalReadSize() + read.getSize());

	return true;
}


unsigned int DataSourceReadSet::getNumReads(void) const
{
	return static_cast<uint>(readSet.size());
}


ptds::DataSourceRead *DataSourceReadSet::getRead(ReadIndex index)
{
	if(index < getNumReads())
	{
		return &(readSet[index]);
	}

	return NULL;
}


const ptds::DataSourceRead *DataSourceReadSet::getRead(ReadIndex index) const
{
	if(index < getNumReads())
	{
		return &(readSet[index]);
	}

	return NULL;
}


void DataSourceReadSet::setTotalItemSize(ItemSize size)
{
	totalItemSize = size;
}


DataSourceReadSet::ItemSize DataSourceReadSet::getTotalItemSize(void)
{
	return totalItemSize;
}


void DataSourceReadSet::setTotalReadSize(DataSize size)
{
	totalReadSize = size;
}


ptds::DataSize DataSourceReadSet::getTotalReadSize(void) const
{
	return totalReadSize;
}


bool DataSourceReadSet::isDefined(void)
{
	return getNumReads() > 0 && getTotalReadSize() > 0;
}


void DataSourceReadSet::write(PTRMI::DataBuffer &buffer) const
{
	unsigned int			numReads;
	unsigned int			t;
	const DataSourceRead *	read;
	DataSize				totalReadSize;

	numReads		= getNumReads();
	totalReadSize	= getTotalReadSize();
															// Write number of reads to be made in total
	buffer << numReads;
															// Write total data size of all reads
	buffer << totalReadSize;
															// For all reads in set
	for(t = 0; t < numReads; t++)
	{
															// Get read
		if(read = getRead(t))
		{
															// Send the data pointer for the read position
			buffer << read->getPosition();
															// Send the read size
			buffer << read->getSize();
		}
	}
}


void DataSourceReadSet::read(PTRMI::DataBuffer &buffer)
{
	unsigned int			numReads;
	DataSize				totalReadSize;
	DataPointer				readPosition;
	DataSize				readSize;
	unsigned int			t;

															// Get total number of reads
	buffer >> numReads;
															// Get total read size
	buffer >> totalReadSize;
															// For all reads
	for(t = 0; t < numReads; t++)
	{
															// Get Data Pointer read position
		buffer >> readPosition;	
															// Get read data size
		buffer >> readSize;
															// Create partial reads
		DataSourceRead read(NULL, 1, readPosition, readSize, NULL);
															// Add read
		addRead(read);
	}

}


DataSource::Data *DataSourceReadSet::executeReadSet(DataSource &dataSource, DataSource::Data *buffer)
{
	if(isDefined() == false)
		return NULL;
															// Executes a local batch read set
	unsigned int			numReads;
	unsigned int			t;
	DataSourceRead		*	read;
	DataSize				numBytesRead;
	DataSource::Data	*	bufferPosition;
	bool					bufferAllocated = false;

															// If no data in read set, return NULL
	if((numReads = getNumReads()) == 0)
		return NULL;
															// If no buffer specified
	if(buffer == NULL)
	{
															// Create a new buffer
		if((buffer = new DataSource::Data[getTotalReadSize()]) == NULL)
			return NULL;

		bufferAllocated = true;
	}

	bufferPosition = buffer;
															// For each read
	for(t = 0; t < numReads; t++)
	{
															// Get the read
		if(read = getRead(t))
		{
															// Read data into the buffer position (contiguously)
			if((numBytesRead = read->read(dataSource, bufferPosition)) != read->getSize())
			{
				if(bufferAllocated)
				{
					delete buffer;
				}

				return NULL;
			}
															// Increment buffer ready for next read
			bufferPosition += numBytesRead;
		}
	}
															// Return the buffer	
	return buffer;
}


DataSource::DataPointer DataSourceReadSet::getFinalDataPointer(void)
{
	DataSourceRead *read;
															// Reproduce the ReadPointer position based on the last Read in the ReadSet
	if(read = getRead(getNumReads() - 1))
	{
															// Final data pointer is position of last read plus the read size
		return (read->getPosition() + read->getSize());
	}
															// Error, so just clear
	return 0;
}


ptds::DataSize DataSourceReadSet::transferVoxelData(DataSource::Data *source)
{
	DataSourceRead					*	read;
	pcloud::Voxel					*	voxel = nullptr;
	DataSource::Data				*	dest;
	unsigned int						numReads;
	ReadIndex							t;
	ptds::DataSize						totalReadSize = 0;

#ifndef NO_DATA_SOURCE_SERVER

															// If no source specified, no data copied
	if(source == NULL)
	{
		return 0;
	}

	numReads = getNumReads();
															// Transfer read data to voxels
	for(t = 0; t < numReads; t++)
	{
															// Get read 
		if((read = getRead(t)) == NULL)
		{
			return 0;
		}
															// Get voxel associated with the read
		if((voxel = reinterpret_cast<pcloud::Voxel *>(read->getClientID())) == NULL)
		{
			return 0;
		}
															// Get the buffer to copy read into (channel buffer)
		if(dest = read->getBuffer())
		{
			std::lock_guard<std::mutex> vlock(voxel->mutex());
															// Copy from main source buffer to voxel channel buffer
															// using read source Windowing if using cache
			memcpy(dest, source, read->getSize());
															// Set current LOD to the achieved Stream LOD
			voxel->setCurrentLOD(voxel->getStreamLOD());
															// Keep track of total amount of data copied
			totalReadSize += read->getSize();
		}
															// Advance source pointer past whole read
		source += read->getSize();
	}

															// Reset each affected voxel's resizedToStream
	for(t = 0; t < numReads; t++)
	{
        std::lock_guard<std::mutex> vlock(voxel->mutex());
															// Get Read
		if(read = getRead(t))
		{
															// Get Read's voxel
			if(voxel = reinterpret_cast<pcloud::Voxel *>(read->getClientID()))
			{
															// Clear resizedToStream
				voxel->setResizedToStream(false);
			}
		}
	}


#endif
															// Return total amount of bytes copied
	return totalReadSize;
}




} // End ptds namespace