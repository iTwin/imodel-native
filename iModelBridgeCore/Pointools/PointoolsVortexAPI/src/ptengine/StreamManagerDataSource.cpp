
#include <ptengine/StreamManagerDataSource.h>


namespace pointsengine
{


StreamManagerDataSource::StreamManagerDataSource(DataSourcePtr initDataSource)
{
	setDataSource(initDataSource);
															// State that ReadSet buffer should manage it's own allocation
	readSetBuffer.createInternalBuffer(1024 * 1024 * 1);
}


StreamManagerDataSource::~StreamManagerDataSource(void)
{
	
}


ptds::DataSize StreamManagerDataSource::executeReadSet(void)
{
	DataSize					totalReadSize;
	PTRMI::DataBuffer::Data	*	buffer;
															// Make sure data source and read set exist
	if(getDataSource() == NULL || getReadSet() == NULL)
		return false;
															// Get read set's total read size (in bytes)	
	if((totalReadSize = getReadSet()->getTotalReadSize()) == 0)
		return 0;
															// Make sure enough buffer space exists	
	if((buffer = readSetBuffer.allocate(totalReadSize)) == NULL)
		return 0;
															// Do all reads in read set as a single batch
															// that reads into the single readSetBuffer
	DataSize sizeRead = getDataSource()->readBytesReadSet(buffer, getReadSet());

// Pip Test
//verifyReadSet(buffer, getReadSet());

	return sizeRead;
}


bool StreamManagerDataSource::beginReadSet(void)
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


bool StreamManagerDataSource::endReadSet(void)
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


DataSourceReadSet * StreamManagerDataSource::getReadSet(void)
{
	return &readSet;
}


void StreamManagerDataSource::setDataSource(DataSourcePtr initDataSource)
{
	dataSource = initDataSource;
}


ptds::DataSourcePtr StreamManagerDataSource::getDataSource(void)
{
	return dataSource;
}


bool StreamManagerDataSource::addVoxel(Voxel *voxel)
{
	if(voxel)
	{
		voxelSet.insert(voxel);

		return true;
	}

	return false;
}


bool StreamManagerDataSource::removeVoxel(Voxel *voxel)
{
	VoxelSet::iterator	it;

	if((it = voxelSet.find(voxel)) != voxelSet.end())
	{
		voxelSet.erase(it);
	}

	return false;
}


void StreamManagerDataSource::clear(void)
{
	voxelSet.clear();

	readSetBuffer.clear();
}


ptds::DataSize StreamManagerDataSource::loadReadSetVoxelData(void)
{
	DataSize							totalReadSize;
	DataSourceRead					*	read;
	DataSourceReadSet::ReadIndex		t;
	Voxel							*	voxel;
	DataSource::Data				*	source;
	DataSource::Data				*	dest;
	unsigned int						numReads;

	if(getDataSource()->getReadSetEnabled() == false || readSet.getNumReads() == 0)
		return 0;
	
	readSetBuffer.setReadPtr(0);
	readSetBuffer.setWritePtr(0);
															// Load all data into the ReadSet buffer
															// Make sure all expected data was read
	if((totalReadSize = executeReadSet()) == 0 || totalReadSize != readSet.getTotalReadSize())
		return false;

	numReads = readSet.getNumReads();
															// Source buffer is the single ReadSet buffer
	source = readSetBuffer.getBuffer();

															// Transfer read data to voxels
	for(t = 0; t < numReads; t++)
	{
															// Get read 
		if((read = readSet.getRead(t)) == NULL)
			return 0;
															// Get voxel associated with the read
		if((voxel = reinterpret_cast<Voxel *>(read->getClientID())) == NULL)
			return 0;
															// Get the buffer to copy read into (channel buffer)
		if(dest = read->getBuffer())
		{
			boost::mutex::scoped_lock vlock(voxel->mutex());

															// If this Read was not cancelled by another thread
			if(voxel->getStreamIterationCancelled() == false)
			{
															// Copy from main source buffer to voxel channel buffer
															// using read source Windowing if using cache
				memcpy(dest, source, read->getSize());
															// Set current LOD to the achieved Stream LOD
				voxel->setCurrentLOD(voxel->getStreamLOD());
			}
			else
			{
															// This read was cancelled during the fetch
															// Restore stream LOD to current LOD
				voxel->setStreamLOD(voxel->getCurrentLOD());
			}
		}
															// Advance source pointer past whole read
		source += read->getSize();
	}

/*
	for(t = 0; t < numReads; t++)
	{
															// Get read 
		if((read = readSet.getRead(t)) == NULL)
			return 0;
															// Get voxel associated with the read
		if((voxel = reinterpret_cast<Voxel *>(read->getClientID())) == NULL)
			return 0;
															// Lock voxel for the life of this scope
		{
			boost::mutex::scoped_lock vlock(voxel->mutex(), boost::defer_lock);

			try 
			{
				if (vlock.owns_lock() == false)
				{
					vlock.lock(); 
															// Set current LOD to the achieved Stream LOD
					voxel->setCurrentLOD(voxel->getStreamLOD());
				}
			}
			catch(...)
			{
			}
		}

	}
*/
															// Clear the read set
	readSet.clear();
															// Return total read size
	return totalReadSize;
}


bool StreamManagerDataSource::verifyReadSet(PTRMI::DataBuffer::Data *buffer, DataSourceReadSet *readSet)
{
	DataSourcePtr	dataSource;
	unsigned int	t;
	unsigned int	i;

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

		DataSize readSize = read->getSize();

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




} // End pointsengine namespace
