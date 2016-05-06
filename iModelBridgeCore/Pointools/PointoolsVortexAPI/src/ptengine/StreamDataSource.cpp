#include "PointoolsVortexAPIInternal.h"

#include <ptengine/StreamHost.h>
#include <ptengine/StreamDataSource.h>
#include <PTRMI/DataBuffer.h>


namespace pointsengine
{

StreamDataSource::StreamDataSource(void)
{
	clear();
}


StreamDataSource::StreamDataSource(DataSourcePtr initDataSource)
{
	setDataSource(initDataSource);

	readSetBuffer.setMode(PTRMI::DataBuffer::Mode_Internal);
}


StreamDataSource::~StreamDataSource(void)
{
	
}


ptds::DataSize StreamDataSource::executeReadSet(void)
{
	ptds::DataSize				totalReadSize;
	PTRMI::DataBuffer::Data	*	buffer;
															// Make sure data source and read set exist
	if(getDataSource() == NULL || getReadSet() == NULL)
		return false;
															// Get read set's total read size (in bytes)	
	if((totalReadSize = getReadSet()->getTotalReadSize()) == 0)
		return 0;
															// Make sure enough buffer space exists	
    if ((buffer = readSetBuffer.allocate(static_cast<PTRMI::DataBuffer::DataSize>(totalReadSize))) == NULL)
		return 0;
															// Do all reads in read set as a single batch
															// that reads into the single readSetBuffer
	ptds::DataSize sizeRead = getDataSource()->readBytesReadSet(buffer, getReadSet());

// Pip Option
//verifyReadSet(buffer, getReadSet());

	return sizeRead;
}


bool StreamDataSource::beginReadSet(void)
{
															// If the data source is defined and this data source uses ReadSets
	if(getDataSource() && getDataSource()->getReadSetEnabled())
	{
															// Begin the reads set
		return getDataSource()->beginReadSet(&readSet);
	}
															// Not configured for ReadSet use
	return false;
}


bool StreamDataSource::endReadSet(void)
{
															// If the data source is defined and this data source uses ReadSets
	if(getDataSource() && getDataSource()->getReadSetEnabled())
	{
															// End the read set
		return getDataSource()->endReadSet(&readSet);
	}
																									// Not configured for ReadSet use
	return false;
}


DataSourceReadSet * StreamDataSource::getReadSet(void)
{
	return &readSet;
}


void StreamDataSource::setDataSource(DataSourcePtr initDataSource)
{
	dataSource = initDataSource;
}


ptds::DataSourcePtr StreamDataSource::getDataSource(void)
{
	return dataSource;
}


bool StreamDataSource::addVoxel(Voxel *voxel)
{
	if(voxel)
	{
		voxelSet.insert(voxel);

		return true;
	}

	return false;
}


bool StreamDataSource::removeVoxel(Voxel *voxel)
{
	VoxelSet::iterator	it;

	if((it = voxelSet.find(voxel)) != voxelSet.end())
	{
		voxelSet.erase(it);
	}

	return false;
}


void StreamDataSource::clear(void)
{
	voxelSet.clear();

	readSetBuffer.clear();
}


ptds::DataSize StreamDataSource::loadReadSetVoxelData(StreamHost *streamHost)
{
	ptds::DataSize	totalReadSize;

	if(getDataSource()->getReadSetEnabled() == false || readSet.getNumReads() == 0)
		return 0;
	
	readSetBuffer.setReadPtr(0);
	readSetBuffer.setWritePtr(0);

															// If StreamHost specified, start it's performance analysis
	if(streamHost)
	{
		streamHost->beginRead(1, readSet.getNumReads(), readSet.getTotalReadSize());
	}
															// Load all data into the ReadSet buffer
															// Make sure all expected data was read
	if((totalReadSize = executeReadSet()) == 0 || totalReadSize != readSet.getTotalReadSize())
		return false;
															// If StreamHost specified, end it's performance analysis
	if(streamHost)
	{
		streamHost->endRead(1, readSet.getNumReads(), readSet.getTotalReadSize());
	}
															// Transfer the ReadSet data to the voxel channels
	readSet.transferVoxelData(readSetBuffer.getBuffer());

															// Clear the read set
	readSet.clear();
															// Return total read size
	return totalReadSize;
}



ptds::DataSize StreamDataSource::transferReadSetVoxelData(DataSourceReadSet &readSet, DataSource::Data *source)
{
	DataSourceRead					*	read;
	Voxel							*	voxel;
	DataSource::Data				*	dest;
	unsigned int						numReads;
	DataSourceReadSet::ReadIndex		t;
	ptds::DataSize						totalReadSize = 0;

															// If no source specified, no data copied
	if(source == NULL)
	{
		return 0;
	}

	numReads = readSet.getNumReads();
															// Transfer read data to voxels
	for(t = 0; t < numReads; t++)
	{
															// Get read 
		if((read = readSet.getRead(t)) == NULL)
		{
			return 0;
		}
															// Get voxel associated with the read
		if((voxel = reinterpret_cast<Voxel *>(read->getClientID())) == NULL)
		{
			return 0;
		}
															// Get the buffer to copy read into (channel buffer)
		if(dest = read->getBuffer())
		{
            std::lock_guard<std::mutex> lock(voxel->mutex());
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
															// Return total amount of bytes copied
	return totalReadSize;
}


bool StreamDataSource::verifyReadSet(PTRMI::DataBuffer::Data *buffer, DataSourceReadSet *readSet)
{
	DataSourcePtr	dataSource;
	unsigned int	t;

	if((dataSource = getDataSource()) == NULL || buffer == NULL || readSet == NULL)
		return false;

	DataSource::Data	*	bufferPosition = buffer;
	DataSource::Data	*	newBuffer;
	DataSourceRead		*	read;

	bool previousReadSetEnabled = dataSource->getReadSetEnabled();

	dataSource->setReadSetEnabled(false);

	for(t = 0; t < readSet->getNumReads(); t++)
	{
		if((read = readSet->getRead(t)) == NULL)
		{
			dataSource->setReadSetEnabled(previousReadSetEnabled);
			return false;
		}

		ptds::DataSize readSize = read->getSize();

		if((newBuffer = new DataSource::Data[readSize]) == NULL)
		{
			dataSource->setReadSetEnabled(previousReadSetEnabled);
			return false;
		}

		dataSource->movePointerTo(read->getPosition());
		dataSource->readBytes(newBuffer, readSize);
		
		if(memcmp(bufferPosition, newBuffer, readSize) != 0)
		{
			dataSource->setReadSetEnabled(previousReadSetEnabled);
			return false;
		}

		bufferPosition += readSize;

		delete []newBuffer;
	}

	dataSource->setReadSetEnabled(previousReadSetEnabled);

	return true;
}


bool StreamDataSource::initializeReadSetBuffer(ptds::DataSize size)
{
															// Initialize internal buffer management with a buffer of given size
	if(readSetBuffer.createInternalBuffer(static_cast<PTRMI::DataBuffer::DataSize>(size)).isOK())
	{
															// Return OK
		return true;
	}
															// Return failed
	return false;
}





} // End pointsengine namespace
