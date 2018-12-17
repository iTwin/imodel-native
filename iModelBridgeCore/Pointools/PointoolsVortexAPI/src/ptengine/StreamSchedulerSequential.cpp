#include "PointoolsVortexAPIInternal.h"

#include <ptengine/StreamSchedulerSequential.h>
#include <ptengine/StreamHost.h>

#include <ptengine/StreamManager.h>

namespace pointsengine
{

const StreamHost::StreamTime STREAM_SCHEDULER_SEQUENTIAL_MINIMUM_PERIOD_TOTAL = 0.01;


bool StreamSchedulerSequential::calculateStreamHostPeriods(StreamManagerParameters &params)
{
	StreamHost::NumVoxels			numVoxelsTotal;
	StreamHost::StreamTime			streamPeriodTotal;
	StreamHostPtrSet::iterator		it;
	StreamHost					*	streamHost;
	StreamHost::StreamTime			periodRatio;
	StreamHost::StreamTime			streamHostPeriod;

	numVoxelsTotal		= getNumVoxelsActive();
	streamPeriodTotal	= params.getStreamPeriodTotal();

	if(numVoxelsTotal == 0)
	{
		return false;
	}
															// Enforce lower time minimum
	streamPeriodTotal = std::max(streamPeriodTotal, STREAM_SCHEDULER_SEQUENTIAL_MINIMUM_PERIOD_TOTAL);

	for(it = streamHostsActive.begin(); it != streamHostsActive.end(); it++)
	{
		if(streamHost = *it)
		{
			periodRatio = static_cast<StreamHost::StreamTime>(streamHost->getNumVoxelsActive()) / static_cast<StreamHost::StreamTime>(numVoxelsTotal);

			streamHostPeriod = params.getStreamPeriodTotal() * periodRatio;

			streamHost->setStreamPeriod(streamHostPeriod);
		}
	}
	
	return true;
}


bool StreamSchedulerSequential::processStreamHosts(StreamManagerParameters &params)
{
	StreamHostPtrSet::iterator		it;
	StreamHost					*	streamHost;

															// If no StreamHosts are currently active, i.e. need data streamed, return
	if(getNumStreamHostsActive() == 0)
	{
		return true;
	}
															// Calculate sequential stream period based on total period and relative approximation of amount of streaming to be done
															// The budget fetched is then dependent on the bandwidth from the StreamHost over the StreamHost's period
	if(calculateStreamHostPeriods(params) == false)
	{
		return false;
	}
															// For each StreamHost that needs data streaming
	for(it = streamHostsActive.begin(); it != streamHostsActive.end(); it++)
	{
															// Get the StreamHost
		if(streamHost = *it)
		{
															// Stream from the StreamHost
			processStreamHost(params, *streamHost);
		}
	}
															// Return OK
	return true;
}


bool StreamSchedulerSequential::processStreamHost(StreamManagerParameters &params, StreamHost &streamHost)
{
	ptds::DataSize				voxelBudgetTotal;
	ptds::DataSize				streamHostBudgetTotal;
	bool						result;

															// Get total target bandwidth budget to be spent on this StreamHost
															// VoxelBudget is thresholded at a lower minimum
	streamHostBudgetTotal = streamHost.getStreamBudget(params, voxelBudgetTotal);
															// If Zero budget specified, do zero data round trip latency sample
	if(streamHostBudgetTotal == 0)
	{
		streamHost.streamRoundTrip();
		return true;
	}

#ifdef PTRMI_LOGGING
	Status::log(L"Stream Host Budget : ", streamHostBudgetTotal);
#endif

	params.setStreamBudgetTotal(streamHostBudgetTotal);
	params.setStreamBudgetPerVoxel(voxelBudgetTotal);
															// If zero, exit now
	if(voxelBudgetTotal == 0)
	{
		return true;
	}

															// Generate all StreamHost ReadSets for all StreamHost DataSources
	generateStreamHostReadSets(streamHost, params);
															// Execute MultiReadSet or ReadSets if Multi not supported
	result = streamHostReads(streamHost);

	return result;
}


bool StreamSchedulerSequential::streamHostReads(StreamHost &streamHost)
{
															// If use of MultiReadSets is enabled (depends on Server compatibility)
															// If first attempt to use fails, this gets disabled
	if(streamHost.getMultiReadSetEnabled())
	{
															// Generate MultiReadSet from ReadSets
		if(generateStreamHostMultiReadSet(streamHost) == false)
		{
			return false;
		}
															// Execute MultiReadSet.
		if(streamStreamHostMultiReadSet(streamHost) == false)
		{
															// MultiReadSet failed, so try ReadSet system instead. This will allow recovery or invalidation of individual data sources. 
			if(streamStreamHostReadSets(streamHost) == false)
			{
				return false;
			}
		}
	}
	else
	{
															// Execute all StreamHost ReadSets
		if(streamStreamHostReadSets(streamHost) == false)
		{
			return false;
		}
	}
															// Return OK
	return true;
}


bool StreamSchedulerSequential::processStreamHostsRead(void)
{
	StreamHost					*	streamHost;
	StreamHostPtrSet::iterator		it;
															// For each StreamHost that needs data streaming
	for(it = streamHostsActive.begin(); it != streamHostsActive.end(); it++)
	{
															// Get the StreamHost
		if(streamHost = *it)
		{
															// Stream from the StreamHost
			processStreamHostRead(*streamHost);
		}
	}

	return true;
}


bool StreamSchedulerSequential::processStreamHostRead(StreamHost &streamHost)
{
															// ReadSets are already generated

															// Execute MultiReadSet or ReadSets if not supported by server
	return streamHostReads(streamHost);
}



} // End pointsengine namespace
