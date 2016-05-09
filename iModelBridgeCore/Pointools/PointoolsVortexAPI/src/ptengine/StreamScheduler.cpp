#include "PointoolsVortexAPIInternal.h"

#include <ptengine/StreamScheduler.h>
#include <ptengine/StreamManager.h>
#include <ptengine/PointsPager.h>
#include <ptengine/engine.h>

namespace pointsengine
{


StreamHost *StreamScheduler::addStreamHost(const PTRMI::Name &hostName)
{
															// If StreamHost does not already exist
	if(getStreamHost(hostName) == NULL)
	{
															// Add stream host
		streamHosts[hostName] = StreamHost();

		StreamHost *streamHost;
															// Get StreamHost just added
		if(streamHost = getStreamHost(hostName))
		{
			if(streamHost->initialize(STREAM_HOST_DEFAULT_MULTI_READ_BUFFER_SIZE))
			{
				return streamHost;
			}
															// Failed to initialize, so delete StreamHost
			deleteStreamHost(hostName);
		}
	}
															// Return not added
	return NULL;
}


bool StreamScheduler::deleteStreamHost(const PTRMI::Name &hostName)
{
	NameStreamHostMap::iterator	it;

	if((it = streamHosts.find(hostName)) != streamHosts.end())
	{
		streamHosts.erase(it);

		return true;
	}

	return false;
}


StreamHost *StreamScheduler::getStreamHost(const PTRMI::Name &hostName)
{
	NameStreamHostMap::iterator	it;
															// Look for StreamHost associated with the given HostName
	if((it = streamHosts.find(hostName)) != streamHosts.end())
	{
															// If found, return StreamDataSourceGroupInfo
		return &(it->second);
	}
															// Not found, so return NULL
	return NULL;
}


bool StreamScheduler::isStreamHostActive(StreamHost *streamHost)
{
	return (streamHostsActive.find(streamHost) != streamHostsActive.end());
}


StreamHost *StreamScheduler::addStreamHostActive(const PTRMI::Name &hostName)
{
	StreamHost *	streamHost;

	if((streamHost = getStreamHost(hostName)) == NULL)
	{
		if((streamHost = addStreamHost(hostName)) == NULL)
		{
			return false;
		}
	}

	streamHostsActive.insert(streamHost);

	return streamHost;
}


StreamDataSource *StreamScheduler::addActiveDataSourceVoxel(DataSourcePtr dataSource, Voxel *voxel, bool *streamDataSourceCreated)
{
	PTRMI::Name				hostName;
	StreamHost			*	streamHost;
	StreamDataSource	*	streamDataSource;

	if(dataSource == NULL || voxel == NULL)
	{
		return NULL;
	}
															// Get DataSource Name (Can be URL or GUID form or both)
	dataSource->getHostName(hostName);
															// If at least either a URL or GUID is not defined
	if(hostName.isPartiallyValid() == false)
	{
															// Return failed
		return NULL;
	}
															// Get or create an active StreamHost
	if((streamHost = addStreamHostActive(hostName)) == NULL)
	{
		return NULL;
	}
															// Add active voxel (and possibly data source if not already added) to StreamHost
	if(streamDataSource = streamHost->addActiveDataSourceVoxel(dataSource, voxel, streamDataSourceCreated))
	{
															// Count number of active voxels
		incrementNumVoxelsActive();

		return streamDataSource;
	}
															// Return failed
	return NULL;
}


void StreamScheduler::clear(void)
{
	clearActive();

	streamHosts.clear();
}


void StreamScheduler::clearActive(void)
{
	StreamHostPtrSet::iterator		it;
	StreamHost					*	streamHost;

	for(it = streamHostsActive.begin(); it != streamHostsActive.end(); it++)
	{
		if(streamHost = *it)
		{
			streamHost->clearActive();
		}
	}

	streamHostsActive.clear();

	clearNumVoxelsActive();
}


void StreamScheduler::beginStreaming(void)
{
	clearActive();
}


void StreamScheduler::endStreaming(void)
{

}


void StreamScheduler::setNumVoxelsActive(unsigned int numVoxels)
{
	numVoxelsActive = numVoxels;
}


unsigned int StreamScheduler::getNumVoxelsActive(void)
{
	return numVoxelsActive;
}


void StreamScheduler::clearNumVoxelsActive(void)
{
	setNumVoxelsActive(0);
}


void StreamScheduler::incrementNumVoxelsActive(void)
{
	setNumVoxelsActive(getNumVoxelsActive() + 1);
}


unsigned int StreamScheduler::getNumStreamHostsActive(void)
{
	return static_cast<uint>(streamHostsActive.size());
}


void StreamScheduler::generateStreamHostReadSets(StreamHost &streamHost, StreamManagerParameters &params)
{
	ptds::DataSize						streamBudgetVoxelUsed;
	ptds::DataSize						streamBudgetTotalUsed		= 0;
	ptds::DataSize						streamBudgetIterationUsed	= 0;
	ptds::DataSize						streamBudgetTotalNotUsed	= 0;
	unsigned int						numVoxelsReceivedBudget		= 0;
	StreamHost::StreamVoxelIterator		v;

															// Initialize StreamHost for streaming. Begin all StreamDataSource ReadSets.
	streamHost.beginStreaming();

															// Iterate over all prioritized voxels in the StreamHost until all voxels processed or budget or number of reads exhausted
	for(v = streamHost.getActiveVoxelBegin(); v != streamHost.getActiveVoxelEnd() && streamBudgetTotalUsed < params.getStreamBudgetTotal() && streamHost.getNumReads() < streamHost.getStreamMaxReadsDefault(); v++)
	{
		StreamVoxel &streamVoxel = const_cast<StreamVoxel &>(*v);

		// need to check for paused state
		if (thePointsPager().isPaused()) break;

															// Read data for voxel
		params.getStreamManager()->processRequestsVoxelStream(streamVoxel.getVoxel(), streamVoxel.getDataSource(), params.getGlobalPagerData(), params.getPager(), params.getStreamBudgetPerVoxel(), streamBudgetVoxelUsed, streamBudgetTotalUsed, streamBudgetIterationUsed, streamBudgetTotalNotUsed, numVoxelsReceivedBudget);
	}
															// End StreamHost streaming. End all StreamDataSource ReadSets.
	streamHost.endStreaming();
}


bool StreamScheduler::generateStreamHostMultiReadSet(StreamHost &streamHost)
{
	return streamHost.generateMultiReadSet();
}


bool StreamScheduler::streamStreamHostReadSets(StreamHost &streamHost)
{
	StreamHost::StreamDataSourceIterator		ds;
	StreamDataSource						*	streamDataSource;

															// For each active StreamDataSource
	for(ds = streamHost.getActiveStreamDataSourceBegin(); ds != streamHost.getActiveStreamDataSourceEnd(); ds++)
	{
		if(streamDataSource = &(ds->second))
		{
															// Read the ReadSet into pre-allocated voxel space
			if(streamDataSource->loadReadSetVoxelData(&streamHost) == 0)
			{
				ptds::DataSource *dataSource = streamDataSource->getDataSource();

															// If data source is not defined or has failed
				if(dataSource == NULL || dataSource->validHandle() == false)
				{
															// Return false and exit
					return false;
				}
			}
		}
	}
															// Return OK
	return true;
}


bool StreamScheduler::streamStreamHostMultiReadSet(StreamHost &streamHost)
{
	return streamHost.loadMultiReadSetVoxelData();
}


bool StreamScheduler::processStreamHostsNonStreamed(StreamManagerParameters &params)
{
	StreamHostPtrSet::iterator					it;
	StreamHost::StreamDataSourceIterator		ds;
	StreamHost								*	streamHost;
	StreamDataSource						*	streamDataSource;

															// For each active StreamHost
	for(it = streamHostsActive.begin(); it != streamHostsActive.end(); it++)
	{
															// Get the StreamHost
		if(streamHost = *it)
		{
															// For each of the StreamHost's active data sources
			for(ds = streamHost->getActiveStreamDataSourceBegin(); ds != streamHost->getActiveStreamDataSourceEnd(); ds++)
			{
															// Get the DataSource
				streamDataSource = &(ds->second);

				params.getStreamManager()->processStreamDataSourceNonStreamed(*streamDataSource, params);
			}
		}
	}

	return true;
}


void StreamScheduler::endStreamHostStreaming(void)
{
	StreamHostPtrSet::iterator		it;
	StreamHost					*	streamHost;

	for(it = streamHostsActive.begin(); it != streamHostsActive.end(); it++)
	{
		if(streamHost = *it)
		{
			streamHost->endStreaming();
		}
	}
}



} // End pointsengine namespace


