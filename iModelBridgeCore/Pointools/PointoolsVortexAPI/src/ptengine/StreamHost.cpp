#include "PointoolsVortexAPIInternal.h"

#include <ptds/GraphManager.h>

#include <ptengine/StreamHost.h>

#include <ptds/DataSource.h>

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <PTRMI/ClientInterface.h>
#include <PTRMI/Manager.h>
#endif

#include <ptengine/StreamManagerParameters.h>


namespace pointsengine
{

#define STREAM_HOST_DEFAULT_READ_SET_BUFFER_SIZE	1024 * 1024 * 1
#define STREAM_HOST_ROUND_TRIP_DATA_SIZE			1024

ptds::DataSize	StreamHost::streamMinDefault		= (1024 * 128);
ptds::DataSize	StreamHost::streamMaxDefault		= (1024 * 1024 * 5);
ptds::DataSize	StreamHost::streamRefreshDefault	= (1024 * 256);
double			StreamHost::streamScalarDefault		= 1.2;
ptds::DataSize	StreamHost::streamMinVoxelBudget	= 512;					// Minimum to assign to Voxel. E.g. 10k voxels = 24*10,000 = 240KB
unsigned int	StreamHost::streamMaxReadsDefault	= 3000;					// Maximum number of reads to perform on one stream iteration
bool			StreamHost::streamMinDefaultEnforce = false;				// Do not enforce minimum streaming budget by default

StreamHost::StreamHost(void)
{
	clear();

	setHostInitialized(false);
	setHostFeatureMultiReadSet(false);
}


void StreamHost::clear(void)
{
	clearActive();
}


void StreamHost::clearActive(void)
{
	streamDataSourcesActive.clear();

	voxelsActive.clear();

	setStreamPeriod(0);
}



ptds::DataSize StreamHost::getStreamBudgetPerPeriod(StreamTime targetStreamPeriod, StreamTime &useStreamPeriod)
{
	ptds::DataSize	s;

	s = analyzer.getReadSizePerPeriod(targetStreamPeriod, useStreamPeriod);

	return s;
}


void StreamHost::setStreamMinVoxelBudget(ptds::DataSize budget)
{
	streamMinVoxelBudget = budget;
}


ptds::DataSize StreamHost::getStreamMinVoxelBudget(void)
{
	return streamMinVoxelBudget;
}


void StreamHost::setStreamMaxReadsDefault(unsigned int maxReads)
{
	streamMaxReadsDefault = maxReads;
}


unsigned int StreamHost::getStreamMaxReadsDefault(void)
{
	return streamMaxReadsDefault;
}


ptds::DataSize StreamHost::getStreamBudget(StreamManagerParameters &params, ptds::DataSource::DataSize &voxelBudget)
{
	ptds::DataSize	budget;
	NumVoxels		numVoxels;
	StreamTime		useStreamPeriod;
															// If no voxels are active, return
	if((numVoxels = getNumVoxelsActive()) == 0)
	{
		return 0;
	}
															// Get stream period target for this Host
															// If Stream Budget not stated as zero, calculate voxel budget
	if((budget = getStreamBudgetPerPeriod(getStreamPeriod(), useStreamPeriod)) > 0)
	{
															// Get number of voxels that fit into the reads budget (one read per channel)
		if((numVoxels = getNumVoxelsActiveInReadBudget(getStreamMaxReadsDefault())) > 0)
		{
															// Evenly distribute budget per active voxel
			voxelBudget = budget / numVoxels;
															// Enforce a minimum voxel budget (may mean not all voxels can be streamed in a single iteration)
			voxelBudget = std::max(voxelBudget, getStreamMinVoxelBudget());

#ifdef PTRMI_LOGGING
			Status::log(L"StreamBudget Voxels Active          : ", getNumVoxelsActive());
			Status::log(L"StreamBudget Voxels In Reads Budget : ", numVoxels);
			Status::log(L"StreamBudget Voxels Budget		  : ", voxelBudget);
#endif

															// Return stream's budget
			return budget;
		}
	}
															// Zero budget returned (request for zero data latency round trip)
	return 0;
}


StreamHost::NumVoxels StreamHost::getNumVoxelsActive(void)
{
	return static_cast<int>(voxelsActive.size());
}


StreamHost::NumVoxels StreamHost::getNumVoxelsActiveInReadBudget(unsigned int maxReads)
{
	unsigned int		numVoxels		= 0;
	unsigned int		numReads		= 0;
	Voxel			*	voxel;

	StreamVoxelPriorityQueue::iterator it;
															// For all active voxels associated with this StreamHost that are in the given read budget
	for(it = voxelsActive.begin(); it != voxelsActive.end() && numReads < maxReads; it++)
	{
		StreamVoxel &streamVoxel = const_cast<StreamVoxel &>(*it);
															// Add number of reads required by voxel (one for each channel)
		if(voxel = streamVoxel.getVoxel())
		{
            std::lock_guard<std::mutex> vlock(voxel->mutex());

			numReads += voxel->getNumChannels();
		}

		numVoxels++;
	}
															// If number of voxels examined exceeds max reads, deduct last voxel as out of budget
	if(numReads > maxReads)
	{
		return numVoxels - 1;
	}
															// Otherwise, return the number of voxels
	return numVoxels;
}


bool StreamHost::beginRead(unsigned int numMultiReads, unsigned int numReads, ptds::DataSource::DataSize numBytes)
{
	return analyzer.beginRead(numMultiReads, numReads, numBytes);
}


bool StreamHost::endRead(unsigned int numMultiReads, unsigned int numReads, ptds::DataSource::DataSize numBytes)
{
	return analyzer.endRead(numMultiReads, numReads, numBytes);
}


void StreamHost::setStreamMinDefault(ptds::DataSource::DataSize min)
{
	streamMinDefault = min;
}


ptds::DataSource::DataSize StreamHost::getStreamMinDefault(void)
{
	return streamMinDefault;
}


void StreamHost::setStreamMinDefaultEnforce(bool enabled)
{
	streamMinDefaultEnforce = enabled;
}


bool StreamHost::getStreamMinDefaultEnforce(void)
{
	return streamMinDefaultEnforce;
}



void StreamHost::setStreamMaxDefault(ptds::DataSource::DataSize max)
{
	streamMaxDefault = max;
}


ptds::DataSource::DataSize StreamHost::getStreamMaxDefault(void)
{
	return streamMaxDefault;
}


void StreamHost::setStreamRefreshDefault(ptds::DataSource::DataSize refresh)
{
	streamRefreshDefault = refresh;
}


ptds::DataSource::DataSize StreamHost::getStreamRefreshDefault(void)
{
	return streamRefreshDefault;
}


void StreamHost::setStreamScalarDefault(float scalar)
{
	streamScalarDefault = scalar;
}


float StreamHost::getStreamScalarDefault(void)
{
	return static_cast<float>(streamScalarDefault);
}


void StreamHost::setStreamPeriod(StreamTime time)
{
	streamTime = time;
}


StreamHost::StreamTime StreamHost::getStreamPeriod(void)
{
	return streamTime;
}


bool StreamHost::beginStreaming(void)
{
	StreamDataSourceIterator		it;
	StreamDataSource			*	streamDataSource;

															// For each active StreamDataSource
	for(it = getActiveStreamDataSourceBegin(); it != getActiveStreamDataSourceEnd(); it++)
	{
		if(streamDataSource = &(it->second))
		{
															// Begin construction of the ReadSet
			if(streamDataSource->beginReadSet() == false)
			{
				return false;
			}
		}
	}

	return true;
}


bool StreamHost::endStreaming(void)
{
	StreamDataSourceIterator		it;
	StreamDataSource			*	streamDataSource;
															// For each active StreamDataSource
	for(it = getActiveStreamDataSourceBegin(); it != getActiveStreamDataSourceEnd(); it++)
	{
		if(streamDataSource = &(it->second))
		{
															// End construction of the ReadSet
			if(streamDataSource->endReadSet() == false)
			{
				return false;
			}
		}
	}


	return true;
}


StreamHost::StreamVoxelIterator StreamHost::getActiveVoxelBegin(void)
{
	return voxelsActive.begin();
}


StreamHost::StreamVoxelIterator StreamHost::getActiveVoxelEnd(void)
{
	return voxelsActive.end();
}


StreamDataSource *StreamHost::getActiveStreamDataSource(ptds::DataSourcePtr dataSource)
{
	StreamDataSourceMap::iterator	it;

	if((it = streamDataSourcesActive.find(dataSource)) != streamDataSourcesActive.end())
	{
		return &(it->second);
	}

	return NULL;
}


unsigned int StreamHost::getNumActiveStreamDataSources(void)
{
	return static_cast<int>(streamDataSourcesActive.size());
}


bool StreamHost::initializeHost(DataSourcePtr dataSource)
{
															// If already initialized, exit with OK
	if(getHostInitialized())
	{
		return true;
	}

	if(dataSource)
	{
															// If not initialized to PTRMI Host
		Name	name;
															// Get it from the DataSource
		dataSource->getHostName(name);
															// Set name in this StreamHost
		setHostName(name);
															// Enable or disable MultiReadSets based on whether server supports it
		bool multiReadSetSupported = getHostFeatureMultiReadSet();
															// Record whether MultiReadSets are supported by remote host
		setHostFeatureMultiReadSet(multiReadSetSupported);
															// Enable or disable MultiReadSet accordingly
		setMultiReadSetEnabled(multiReadSetSupported);
															// Flag as initialized
		setHostInitialized(true);
															// Return OK
		return true;
	}
															// Returned failed
	return false;
}


StreamDataSource *StreamHost::getOrCreateActiveStreamDataSource(ptds::DataSourcePtr dataSource, bool *streamDataSourceCreated)
{
	StreamDataSource *streamDataSource;

	if(dataSource == NULL)
	{
		return NULL;
	}

	if(streamDataSourceCreated)
	{
		*streamDataSourceCreated = false;
	}

															// Initialize host for use with PTRMI Host if necessary
	initializeHost(dataSource);


	if((streamDataSource = getActiveStreamDataSource(dataSource)) == NULL)
	{
		if(streamDataSourceCreated)
		{
			*streamDataSourceCreated = true;
		}

        streamDataSourcesActive.insert({dataSource,StreamDataSource(dataSource)});

		if((streamDataSource = getActiveStreamDataSource(dataSource)) == NULL)
		{
			return NULL;
		}
		
		streamDataSource->initializeReadSetBuffer(STREAM_HOST_DEFAULT_READ_SET_BUFFER_SIZE);
	}

	return streamDataSource;
}


bool StreamHost::isDataSourceValid(ptds::DataSourcePtr dataSource)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	ClientInterfaceBase *dataSourceClientInterface;
															// If DataSource is networked
	if(dataSource->getDataSourceForm() == DataSource::DataSourceFormRemote)
	{
															// Get DataSource's ClientInterface		
		if(dataSourceClientInterface = dynamic_cast<ClientInterfaceBase *>(dataSource))
		{
															// Return OK if ClientInterface is still valid
			return dataSourceClientInterface->getStatus().isOK();
		}
	}
#endif

															// Return OK
	return true;
}


void StreamHost::setHostFeatureMultiReadSet(bool supported)
{
	hostFeatureMultiReadSet = supported;
}


bool StreamHost::getHostFeatureMultiReadSet(void)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
    bool result;
#endif

	if(getHostInitialized())
	{
		return hostFeatureMultiReadSet;
	}
	else
	{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
		if(PTRMI::getManager().getHostFeatureMultiReadSet(getHostName(), result))
		{
			return result;
		}
#endif
	}

	return false;
}


StreamDataSource *StreamHost::addActiveDataSourceVoxel(ptds::DataSourcePtr dataSource, pcloud::Voxel *voxel, bool *streamDataSourceCreated)
{
	StreamDataSource *streamDataSource;

															// If DataSource is invalid, don't use it
	if(isDataSourceValid(dataSource) == false)
	{
		return NULL;
	}

	if(streamDataSourceCreated)
	{
		*streamDataSourceCreated = false;
	}

#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// If DataSource is networked
	if(dataSource->getDataSourceForm() == DataSource::DataSourceFormRemote)
	{
		ClientInterfaceBase *dataSourceClientInterface;
															// Get DataSource's ClientInterface		
		if(dataSourceClientInterface = dynamic_cast<ClientInterfaceBase *>(dataSource))
		{
															// If DataSource has failed
			if(dataSourceClientInterface->getStatus().isFailed())
			{
															// Don't add, return false
				return NULL;
			}
		}
	}
#endif
															// Get or create an active StreamDataSource object associated and create a StreamVoxel entry
	if((streamDataSource = getOrCreateActiveStreamDataSource(dataSource, streamDataSourceCreated)) == NULL)
	{
		return NULL;
	}
															// Add a StreamVoxel object bound to the StreamDataSource and the Voxel
	voxelsActive.insert(StreamVoxel(streamDataSource, voxel));
															// Return OK
	return streamDataSource;
}


StreamHost::StreamDataSourceIterator StreamHost::getActiveStreamDataSourceBegin(void)
{
	return streamDataSourcesActive.begin();
}


StreamHost::StreamDataSourceIterator StreamHost::getActiveStreamDataSourceEnd(void)
{
	return streamDataSourcesActive.end();
}


bool StreamHost::generateMultiReadSet(void)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB
	StreamDataSourceMap::iterator		it;
	DataSourceReadSet				*	readSet;
	ClientInterfaceBase				*	dataSourceClientInterface;
	PTRMI::GUID							dataSourceServerInterface;
	unsigned int						numAdded = 0;

															// Make sure MultiReadSet is empty
	multiReadSet.clear();
															// For each active data source in Host
	for(it = streamDataSourcesActive.begin(); it != streamDataSourcesActive.end(); it++)
	{
															// Get DataSource's ClientInterface if it is a networked type
		if(dataSourceClientInterface = dynamic_cast<ClientInterfaceBase *>(it->first))
		{
															// Get DataSource's StreamDataSource
			StreamDataSource &streamDataSource = it->second;
															// Get StreamDataSource's ReadSet
			if(readSet = streamDataSource.getReadSet())
			{
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
															// Get GUID of remote DataSource object
				dataSourceServerInterface = dataSourceClientInterface->getRemoteInterface().getGUID();
															// Add MultiRead (ReadSet)
				if(multiReadSet.addMultiRead(DataSourceMultiRead(*readSet, dataSourceServerInterface)))
				{
					++numAdded;
				}
#endif
			}
		}
	}
															// Return whether correct number were added
	return (numAdded == multiReadSet.getNumMultiReads());
#else
    return false;
#endif
}


StreamDataSource *StreamHost::getFirstActiveStreamDataSource(void)
{
	StreamDataSourceMap::iterator	it;

	if((it = streamDataSourcesActive.begin()) != streamDataSourcesActive.end())
	{
		return &(it->second);
	}

	return NULL;
}


ptds::DataSize StreamHost::executeMultiReadSet(void)
{
    ptds::DataSize		        totalReadSize;
	PTRMI::DataBuffer::Data	*	buffer;
	ptds::DataSize				sizeRead;
	StreamDataSource		*	streamDataSource;
	DataSource				*	dataSource;
	//DataSourceMultiRead		*	firstMultiRead;
															// Get read set's total read size (in bytes)	
	if((totalReadSize = multiReadSet.getTotalReadSize()) == 0)
		return 0;
															// Make sure enough buffer space exists	
	if((buffer = multiReadSetBuffer.allocate(static_cast<PTRMI::DataBuffer::DataSize>(totalReadSize))) == NULL)
		return 0;
															// Get the first active StreamDataSource to carry out the remote operation
	if((streamDataSource = getFirstActiveStreamDataSource()) == NULL)
		return 0;
															// Get the first DataSource
	if((dataSource = streamDataSource->getDataSource()) == NULL)
		return 0;
															// Do all reads in read set as a single batch
															// that reads into the single readSetBuffer
	sizeRead = dataSource->readBytesMultiReadSet(buffer, &multiReadSet);

	return sizeRead;
}

bool StreamHost::loadMultiReadSetVoxelData(void)
{
	ptds::DataSize	totalReadSize;
	ptds::DataSize	totalSizeRead;
	unsigned int	numMultiReads;
	unsigned int	numReads;

	multiReadSetBuffer.setReadPtr(0);
	multiReadSetBuffer.setWritePtr(0);
															// If no work to do, return OK
	if((totalReadSize = multiReadSet.getTotalReadSize()) == 0)
	{
		return true;
	}

	numMultiReads	= multiReadSet.getNumMultiReads();
	numReads		= multiReadSet.getNumReads();

	analyzer.beginPeriod();
															// Start StreamHost's performance analysis
	beginRead(numMultiReads, numReads, totalReadSize);
															// Load all data into the MultiReadSet buffer
															// Make sure all expected data was read
	totalSizeRead = executeMultiReadSet();
															// End StreamHost's performance analysis
	endRead(numMultiReads, numReads, totalReadSize);

															// Make sure the right amount of data was read
	if(totalSizeRead == 0 || totalSizeRead != totalReadSize)
	{
#ifdef PTRMI_LOGGING
		Status status(Status::Status_Error_Reading_Multi_Read_Set);
		Status::log(L"Warning: StreamHost::loadMultiReadSetVoxelData() MultiReadSet failed", L"");
#endif
		return false;
	}

	DataSourceMultiReadSet::MultiReadIndex		t;
	DataSourceMultiRead						*	multiRead;
	DataSource::Data						*	source;
	DataSourceReadSet						*	readSet = nullptr;

															// Get MultiReadSet buffer
	source = multiReadSetBuffer.getBuffer();
															// For each MultiRead (ReadSet)
	for(t = 0; t  < numMultiReads; t++)
	{
															// Get MultiRead (ReadSet)
		if(multiRead = multiReadSet.getMultiRead(t))
		{
			if(readSet = multiRead->getDataSourceReadSet())
			{
															// Transfer the ReadSet data to the voxel channels
				readSet->transferVoxelData(source);
			}
		}
															// Advance source past read
		source += readSet->getTotalReadSize();
	}


	analyzer.endPeriod();

	return true;
}


bool StreamHost::initialize(ptds::DataSize multiReadSetBufferSize)
{

	analyzer.initialize();
															// Initialize internal buffer management with a buffer of given size
	if(multiReadSetBuffer.createInternalBuffer(static_cast<PTRMI::DataBuffer::DataSize>(multiReadSetBufferSize)).isOK())
	{
															// Return OK
		return true;
	}
															// Return failed
	return false;
}


void StreamHost::setMultiReadSetEnabled(bool enabled)
{
	multiReadSet.setEnabled(enabled);
}


bool StreamHost::getMultiReadSetEnabled(void)
{
	return multiReadSet.getEnabled();
}


void StreamHost::setHostInitialized(bool value)
{
	hostInitialized = value;
}


bool StreamHost::getHostInitialized(void)
{
	return hostInitialized;
}


void StreamHost::setHostName(const PTRMI::Name &name)
{
	hostName = name;
}


const PTRMI::Name &StreamHost::getHostName(void)
{
	return hostName;
}


bool StreamHost::streamRoundTrip(void)
{
	StreamDataSource *streamDataSource;

	if(streamDataSource = getFirstActiveStreamDataSource())
	{
		unsigned char		buffer[STREAM_HOST_ROUND_TRIP_DATA_SIZE];
		DataSource		*	dataSource;

		if(dataSource = streamDataSource->getDataSource())
		{
			analyzer.beginRead(1, 1, 0);
            ptds::DataSource::Size r = dataSource->readBytes(buffer, STREAM_HOST_ROUND_TRIP_DATA_SIZE);
            UNUSED_VARIABLE(r);
			analyzer.endRead(1, 1, 0);

			return true;
		}
	}

	return false;
}


unsigned int StreamHost::getNumMultiReads(void)
{
	return multiReadSet.getNumMultiReads();
}


unsigned int StreamHost::getNumReads(void)
{
	return multiReadSet.getNumReads();
}


ptds::DataSize StreamHost::getTotalReadSize(void)
{
	return multiReadSet.getTotalReadSize();
}


} // End pointsengine namespace

