#include "PointoolsVortexAPIInternal.h"

#include <ptengine/StreamManager.h>

#include <ptengine/VoxelLoader.h>

#include <pt/timestamp.h>

#include <PTRMI/Status.h>

#include <ptengine/StreamSchedulerSequential.h>

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <PTRMI/Manager.h>
#endif


#define PTRMI_STREAM_MANAGER_MUTEX_TIMEOUT	1000 * 60

using namespace pointsengine;

// Pip Option
pt::SimpleTimer					test_timer[50];

// Pip Option
bool							testLastRefreshInit = false;
pt::TimeStamp					testLastRefresh;

pointsengine::StreamManager		streamManager;


namespace pointsengine
{

StreamManager &getStreamManager(void)
{
	return streamManager;
}


StreamManager::StreamManager(void)
{
															// Initially no StreamScheduler
	setStreamScheduler(NULL);
															// Initially not in begin/end phase
	setBeginEnd(false);

	clear();
}


StreamManager::~StreamManager(void)
{
	MutexScope mutexScope(streamManagerMutex);

	if(getStreamScheduler() != NULL)
	{
		delete getStreamScheduler();
		setStreamScheduler(NULL);
	}

}


void StreamManager::clear(void)
{
	setCurrentIteration(0);

	if(getStreamScheduler())
	{
		getStreamScheduler()->clear();
	}
}


void StreamManager::clearActive(void)
{
	if(getStreamScheduler())
	{
		getStreamScheduler()->clearActive();
	}
}

#ifdef _XX

int StreamManager::getVoxelReadInfo(PointsPager::Pager *pager, pcloud::Voxel *vox, bool full, bool lock)
{
	if(pager == NULL || vox == NULL)
		return 0;

	int lod_points = vox->getRequestLOD() * vox->fullPointCount();
	int req_points = vox->fullPointCount();
	int curr_points = vox->lodPointCount();

	/*load in extra data required*/
	int file = vox->fileIndex();
	int bytes_loaded = 0;

	/*check for null file index*/ 
	if (file == 255) { /*open scene file*/ }


	if(openFile(vox, _thread))
	{
		/*load data into channels*/ 
		int points;
		uint bytes, offset, prev_channel_size;

		prev_channel_size = 0;
		
		uint chfilter = pointsFilteringState().displayChannelFilter();
		bool dofilter = pointsFilteringState().filterPagingByDisplay();
		uint channelbit = 1;

        std::lock_guard<std::mutex> vlock(vox->mutex(), std::defer_lock);

		try 
		{
			if (lock && !vlock.owns_lock()) vlock.lock(); 
		}
		catch(...)
		{
			return 0;
		}
		int actual_points = 0;

		{
			float save_req = vox->getRequestLOD();
			if (full)	vox->setRequestLOD(1.0f);

			for (int c=1; c<MAX_LOAD_CHANNELS; c++)
			{
				pcloud::DataChannel *dc = const_cast<pcloud::DataChannel*>(vox->channel(c));

				if (!dc || c == pcloud::PCloud_Grid)
					continue;

				pcloud::Channel ch = pcloud::channel(c);
				
				/*reallocate voxel data channels*/ 
				int64_t pos = vox->filePointer() + prev_channel_size;
				
				/*this is the size of full channel data on disk*/ 
				prev_channel_size += vox->fullPointCount() * dc->typesize() * dc->multiple();

				/*can't do this earlier because we need offset value*/ 
				if (dofilter && chfilter & channelbit)
				{
					channelbit <<= 1;
					continue;
				}
				channelbit <<= 1;
				
				points = 0;

#if 0
				// Resizing the channel then filling it with data in two stages is causing errors due to
				// incorrect LOD being stored in the voxel when the second (fill) stage fails. Refactoring
				// to have the resize and fill done in one step
				void *data = vox->resizeChannelToRequestedLod( pcloud::channel(c), 1.0f, actual_points, bytes, offset);			
				
				/* if this fails we're running out of mem */ 
				if (actual_points == -1000) return -1000;

				pos += offset;
				if (data && bytes && pp.files[file].handle[_thread]->movePointerTo(pos))
				{
#define BLOCK_LOAD
#ifdef				BLOCK_LOAD
					static ubyte *sblock=0;
					if (!_buffer && !sblock) sblock = new ubyte[LOAD_BLOCK_SIZE];

					ubyte *block = _buffer ? (ubyte*)_buffer : sblock;

					int bytesNeeded = bytes;
					int bytesRead;
					
					/* load in 64Kb blocks */ 
					for (int i=0; i< (bytes/LOAD_BLOCK_SIZE)+1; i++)
					{
						bytesRead = pp.files[file].handle[_thread]->readBytes(block, LOAD_BLOCK_SIZE);
						memcpy( &((ubyte*)data)[i*LOAD_BLOCK_SIZE], 
							block, 
							(bytesNeeded > LOAD_BLOCK_SIZE ? LOAD_BLOCK_SIZE : bytesNeeded % LOAD_BLOCK_SIZE));

						bytesNeeded -= LOAD_BLOCK_SIZE;
					}
#else
					ptds::IO::readBytes( pp.files[file].handle[_thread], data, bytes);
#endif					
					bytes_loaded += bytes;
				}
#else
				if (vox->resizeAndFillChannelToRequestedLod(pcloud::channel(c), 1.0f, actual_points, bytes, offset, pp.files[file].handle[_thread], pos, _buffer))
				{
					bytes_loaded += bytes;
				}
				else
				{
					return 0;
				}

				/* if this fails we're running out of mem */ 
				if (actual_points == -1000)
				{
					return -1000;
				}
#endif
				if (full)
				{
					vox->setRequestLOD(save_req);
				}
			}
		}
	}

	return bytes_loaded;

}

#endif


void StreamManager::calculateLODChanges(float &am, Voxel * voxel, Voxel::LOD &req, Voxel::LOD &lod, bool &ooc, bool &new_ooc, bool &vis, MemMode memoryMode, Voxel::LOD &ami)
{
	am = voxel->getRequestLODMax();		

	req		= am;
	lod		= voxel->getCurrentLOD();	

	// if we have very low lod but its still more than the threshold number of points
	// important to do load

// Pip Option - Removed low LOD handling because shouldn't be necessary with float based LOD values
/*
	if (am > 0 && !req && voxel->fullPointCount() * am > LOW_LOD_LOAD_THRESHOLD)
	{
		req = 1;				// handle very low lod situation

		if (voxel->getCurrentLOD() > 0 && !lod) 
			lod = 1;	
	}
*/

	ooc = voxel->flag( pcloud::OutOfCore );		// this leaf is OOC
	new_ooc = ooc;						
	vis = voxel->flag( pcloud::Visible );		// is visible

	// load logic
	applyMemoryMode(memoryMode, ooc, new_ooc, req, lod, am, vis, ami);
}


bool StreamManager::applyMemoryMode(MemMode memoryMode, bool ooc, bool &new_ooc, Voxel::LOD req, Voxel::LOD lod, float &am, bool vis, Voxel::LOD &ami)
{
	bool	result = false;

	switch (memoryMode)
	{

	case PointsPager::MemPlenty:

		if (ooc)
		{
			new_ooc = false;
		}
		if( req < lod ) 
		{
			am = -1; // prevent unload
		}
		break;

	case PointsPager::MemOK:

		if(ooc && req > static_cast<Voxel::LOD>(0.2))
		{
			new_ooc = false;
		}
		else
		if(req < lod) 
		{
			am = -1; // prevent unload
		}
		break;

	case PointsPager::MemPrudent:

		if (!vis && req > LOD_ZERO) 
		{
			am = LOD_ZERO;
		}
		else
		if (ooc && req > static_cast<Voxel::LOD>(0.75))
		{
			new_ooc = false;			
		}
		else
		if (req == lod)
		{
			result = true;
		}

		break;

	case PointsPager::MemUrgent:

		if (!vis && req > LOD_ZERO) 
		{
			am = LOD_ZERO;
		}
		else
		if (!ooc || lod > static_cast<Voxel::LOD>(0.5))
		{
			new_ooc = true;

			am = LOD_ZERO; //0.5;
		}
		// CHECK LOCKing
		else
		if (req == lod)
		{
			result = true;
		}

		break;

	case PointsPager::MemCritical:

		if(!ooc || lod > LOD_ZERO)
		{
			new_ooc = true;
			am = LOD_ZERO;
		}
		else
		{
			result = true;
		}

		break;
	}


	ami = am;

	if (am > LOD_ZERO && ami == 0)
		ami = LOD_USE_MINIMUM;

	return result;
}



ptds::DataSize StreamManager::getVoxelStreamRequirement(Voxel *voxel, DataSourcePtr dataSource)
{
	return 0;
}


ptds::DataSize StreamManager::getTotalStreamRequirement(Voxel *voxel, DataSourcePtr dataSource)
{
	return 0;
}


ptds::DataSize StreamManager::getVoxelStreamBudgetUniform(StreamHost &streamHost, ptds::DataSize streamBudgetTotal)
{
	unsigned int	numGroupVoxels;
															// Get number of visible voxels associated with this data source group
	if((numGroupVoxels = streamHost.getNumVoxelsActive()) > 0)
	{
															// Conservatively divide total budget by number of visible voxels in this data source
		return streamBudgetTotal / numGroupVoxels;
	}
															// Division by zero error, so return zero budget
	return 0;
}


ptds::DataSize StreamManager::getVoxelStreamBudgetUniform(StreamDataSource &dataSourceStream, ptds::DataSize streamBudgetTotal)
{

	unsigned int	numDataSourceVoxels;
															// Get number of visible voxels associated with this data source
	if((numDataSourceVoxels = dataSourceStream.getNumVoxels()) > 0)
	{
															// Conservatively divide total budget by number of visible voxels in this data source
		return streamBudgetTotal / numDataSourceVoxels;
	}
															// Division by zero error, so return zero budget
	return 0;

}


PTRMI::Status StreamManager::processStreamDataSourceNonStreamed(StreamDataSource &dataSourceStream, StreamManagerParameters &parameters)
{

	Voxel			*	voxel;
	float				am = 0;
	Voxel::LOD			lod, req;
	pt::TimeStamp		t0;
	int64_t				memAval;

	bool				ooc;		// this leaf is OOC
	bool				new_ooc;						
	bool				vis;		// is visible
	Voxel::LOD			ami = 0;

	PTRMI::Status		status;

#ifdef _TEST
	PTRMI::Status::log(L"processStreamDataSourceNonStreamed", L"Begin");
#endif

															// Exit if there are no voxels to process
	if (dataSourceStream.getNumVoxels() == 0)
		return status;

#ifdef _TEST
	PTRMI::Status::log(L"processStreamDataSourceNonStreamed", L"1");
#endif

															// SET UP ITERATION OVER LEAF LIST IN DEPTH ORDER
	StreamDataSource::VoxelSet::iterator	start, end, i;

	dataSourceStream.getVoxelIterators(start, end);


	t0.tick();												// start timer

	for (i = start; i != end; i++)
	{
		if(voxel = *i)
		{
															// Calculate LOD related variables
			calculateLODChanges(am, voxel, req, lod, ooc, new_ooc, vis, parameters.getMemoryMode(), ami);

															// If LOD should be decreased
			if(unloadLOD(voxel, am, ami, lod, ooc, new_ooc)) 
			{
															// Unload voxel data
				unload(voxel, new_ooc, am, parameters.getGlobalPagerData(), parameters.getPager());
			}
			else
			if(loadLOD(voxel, ami, lod, ooc, new_ooc))		// If LOD should be increased
			{
															// Check for pause or quit flags
				if (!parameters.getPager().checkFlags())
					return status;
															// Load voxel data
				load(voxel, new_ooc, am, parameters.getGlobalPagerData(), parameters.getPager(), am);
			}
		}


		pt::TimeStamp t1;

		t1.tick();

		if (pt::TimeStamp::delta_ms(t0,t1) > 2000)
			break;
															//check for pause or quit flags
		if (!parameters.getPager().checkFlags())
			return status;	

		if (parameters.getGlobalPagerData().iteration % 30 == 0) 
		{
			parameters.getGlobalPagerData().memoryUsed = memoryUsage();
		}

// 		if (!voxelList.size())
// 			return true;

		parameters.setMemoryMode(determineMemoryMode(memAval));

		++(parameters.getGlobalPagerData().iteration);
	}

#ifdef _TEST
	PTRMI::Status::log(L"processRequestsNoStream", L"End");
#endif

	return status;
}


bool StreamManager::processRequestsStreamed(StreamManagerParameters &streamManagerParameters)
{
	StreamManagerDataSourceSet::iterator	i;
	StreamScheduler						*	scheduler;

															// If StreamScheduler does not exist, default to a Sequential stream scheduler
	if((scheduler = getOrCreateStreamScheduler()) == NULL)
	{
		return false;
	}
															// Exit if there are no voxels to process
	if (streamManagerParameters.getVoxelList().size() == 0)
	{
		return false;
	}

															// Increment stream manager iteration
	incrementCurrentIteration();
															// Analyze visible voxels and construct sets of active StreamHosts and DataSources
															// OR immediately process non streamed DataSources
	if(initializeStreamHostDataSourceVoxels(streamManagerParameters, getCurrentIteration()))
	{
															// Process streaming based on the current StreamScheduler
		scheduler->processStreamHosts(streamManagerParameters);
	}

	return true;
}


void StreamManager::processRequestsVoxelStream(Voxel *voxel, DataSourcePtr dataSource, GlobalPagerData &globalPagerData, PointsPager::Pager &pager, ptds::DataSize streamBudgetVoxel, ptds::DataSize &streamBudgetVoxelUsed, ptds::DataSize &streamBudgetTotalUsed, ptds::DataSize &streamBudgetIterationUsed, ptds::DataSize &streamBudgetTotalNotUsed, unsigned int &numVoxelsReceivedBudget)
{
	bool new_ooc = voxel->new_ooc;
	float am = voxel->am;

	voxel->flag(pcloud::OutOfCore, new_ooc);
	voxel->setRequestLOD(am);
															// If increasing LOD
	if(voxel->compareLOD(voxel->getRequestLODMax(), voxel->getCurrentLOD()) == Voxel::LODGreater)
	{
															// Load as much as possible from given budget
		loadStreamBudget(voxel, dataSource, new_ooc, am, globalPagerData, pager, streamBudgetVoxel, streamBudgetVoxelUsed);

		if(streamBudgetVoxel > 0)
		{
			streamBudgetTotalUsed		+= streamBudgetVoxelUsed;
			streamBudgetIterationUsed	+= streamBudgetVoxelUsed;
			streamBudgetTotalNotUsed	+= streamBudgetVoxel - streamBudgetVoxelUsed;
															// Data read, so set iteration counter to this iteration
			voxel->setLastStreamManagerIteration(getCurrentIteration());
															// Increment count of number of voxels served in this iteration
			++numVoxelsReceivedBudget;
		}
	}
}


bool StreamManager::unloadLOD(Voxel *voxel, float am, Voxel::LOD ami, Voxel::LOD lod, bool ooc, bool new_ooc)
{
	return (am >= 0 && voxel->compareLOD(ami, lod) == Voxel::LODLess) || (!ooc && new_ooc);

//	return (am >= 0 && ami < lod) || (!ooc && new_ooc);
}


bool StreamManager::loadLOD(Voxel *voxel, float ami, Voxel::LOD lod, bool ooc, bool new_ooc)
{
	return (voxel->compareLOD(ami, lod) == Voxel::LODGreater) || (ooc && !new_ooc);

//	return ami > lod || (ooc && !new_ooc);
}


void StreamManager::unload(Voxel *voxel, bool new_ooc, float am, GlobalPagerData &globalPagerData, PointsPager::Pager &pager)
{
    std::unique_lock<std::mutex>  vlock( voxel->mutex(), std::try_to_lock );

	if (vlock.owns_lock())
	{
		voxel->flag( pcloud::OutOfCore, new_ooc );

		if (am >= 0)
		{
			globalPagerData.memoryUsed -= pager.VoxelLoader::unloadVoxel(voxel, am, false);
		}
#ifdef _VERBOSE
		std::cout << "-";
#endif
	}
}


void StreamManager::load(Voxel *voxel, bool new_ooc, float am, GlobalPagerData &globalPagerData, PointsPager::Pager &pager, float lodRead)
{
    std::unique_lock<std::mutex> vlock(voxel->mutex(), std::try_to_lock );

	if (vlock.owns_lock())
	{
															// Set new out of core setting
		voxel->flag(pcloud::OutOfCore, new_ooc);
															// Set LOD request
		voxel->setRequestLOD(am);

		Voxel::LOD	req = voxel->getRequestLODMax();
		Voxel::LOD	lod = voxel->getCurrentLOD();

/*
		if (req < 1)
		{
			req = 1;	// low lod handling
		}
*/

		if (req > lod)
		{
			uint bytesLoaded = pager.loadVoxel(voxel, lodRead, false);

#ifdef _VERBOSE
			std::cout << "+";
#endif

			if (bytesLoaded >= 0)
			{
				globalPagerData.memoryUsed += bytesLoaded;
			}
			else // memory failure
			{
				int rb = voxel->requestBytes();

				pager.purgeData(rb * 2);

				bytesLoaded = pager.loadVoxel(voxel, lodRead, false);

				if (bytesLoaded > 0)
				{
					globalPagerData.memoryUsed += bytesLoaded;
				}
			}
		}
	}
}


void StreamManager::loadStreamBudget(Voxel *voxel, DataSourcePtr dataSource, bool new_ooc, float am, GlobalPagerData &globalPagerData, PointsPager::Pager &pager, ptds::DataSize streamBudgetVoxel, ptds::DataSize &streamBudgetUsed)
{
															// If no stream budget, just exit
	if(voxel == NULL)
		return;

	uint64_t	numPointsInBudget;
	float				streamLOD;
	DataSourceReadSet	readSetRequest;
															// Get channel read requests as a read set for full read
															// Streaming is based on partial fulfillment of this within the budget
	voxel->getVoxelDataSourceReadSet(voxel->getRequestLODMax(), readSetRequest);

															// See how many points data source can provide with given budget
															// This differs, primarily between normal and cached data sources
															// Cached data source only uses budget for remote reads
	if(numPointsInBudget = dataSource->getBudgetParallelRead(streamBudgetVoxel, readSetRequest, streamBudgetUsed))
	{
															// Get minimum of potential that can be streamed or current request
		streamLOD = std::min(voxel->getPotentialLOD(numPointsInBudget), voxel->getRequestLODMax());
															// If there's LOD to be loaded
		if(streamLOD > 0)
		{
															// Notify that any read set additions are for this voxel
			dataSource->beginReadSetClientID(voxel);
															// Load to streamLOD only
			load(voxel, new_ooc, am, globalPagerData, pager, streamLOD);
															// Clear ID so reads are not associated with this voxel any longer
			dataSource->endReadSetClientID();
		}
	}

}


bool StreamManager::initializeStreamHostDataSourceVoxels(StreamManagerParameters &streamManagerParameters, Iteration iteration)
{
	PointsScene::VoxIterator		i;
	DataSourcePtr					dataSource;
	Voxel						*	voxel;
	pt::TimeStamp					t0, t1;

	float							am = 0;
	Voxel::LOD						lod, req;
	bool							ooc;					// this leaf is OOC
	bool							new_ooc;						
	bool							vis;					// is visible
	Voxel::LOD						ami = 0;

	VoxelList						streamedVoxels;

															// Get start time
	t0.tick();

															// For all voxels (in depth order ?)
	for(i = streamManagerParameters.getVoxelList().begin(); i != streamManagerParameters.getVoxelList().end(); i++)
	{
		voxel = *i;

		t1.tick();
															// If too long has been spent loading from disk, exit so that visibility calculations can recalculate voxel load order
		if(pt::TimeStamp::delta_ms(t0, t1) > 2000)
			return false;

															// Calculate LOD related variables
		calculateLODChanges(am, voxel, req, lod, ooc, new_ooc, vis, streamManagerParameters.getMemoryMode(), ami);

		if(unloadLOD(voxel, am, ami, lod, ooc, new_ooc)) 
		{
															// Unload voxel data
			unload(voxel, new_ooc, am, streamManagerParameters.getGlobalPagerData(), streamManagerParameters.getPager());
															// Record for later use
			voxel->am = am;
			voxel->new_ooc = new_ooc;
		}
		else
		if(loadLOD(voxel, ami, lod, ooc, new_ooc))			// If LOD should be increased
		{

			voxel->am = am;
			voxel->new_ooc = new_ooc;
															// Check for pause or quit flags
			if (!streamManagerParameters.getPager().checkFlags())	
				return false;
															// If requested LOD is greater (may not be)
			if(voxel->compareLOD(voxel->getRequestLODMax(), voxel->getCurrentLOD()) == Voxel::LODGreater)
			{

				if((dataSource = getVoxelDataSource(voxel, streamManagerParameters.getGlobalPagerData(), streamManagerParameters.getPager().getThread())) == NULL)
				{
															// Attempt to open data source
					if((dataSource = streamManagerParameters.getPager().openFile(voxel, streamManagerParameters.getPager().getThread())) == NULL)
						return false;
				}
															// Only include valid data sources
				if(dataSource->validHandle())
				{
															// If non streamed
					if(dataSource->getReadSetEnabled() == false)
					{
															// Read now without streaming
						load(voxel, new_ooc, am, streamManagerParameters.getGlobalPagerData(), streamManagerParameters.getPager(), am);
					}
					else
					{
						streamedVoxels.push_back(voxel);
					}
				}
			}
		}
	}


															// For all voxels with increasing LOD that must be streamed
	for(i = streamedVoxels.begin(); i != streamedVoxels.end(); i++)
	{
		voxel = *i;

		if((dataSource = getVoxelDataSource(voxel, streamManagerParameters.getGlobalPagerData(), streamManagerParameters.getPager().getThread())) == NULL)
		{
															// Attempt to open data source
			if((dataSource = streamManagerParameters.getPager().openFile(voxel, streamManagerParameters.getPager().getThread())) == NULL)
				return false;
		}
															// Only include valid data sources
		if(dataSource->validHandle())
		{
															// DataSource is streamed. Create a StreamDataSource and StreamVoxel for the StreamHost.
			if(addStreamedVoxel(dataSource, voxel) == false)
			{
				return false;
			}
		}
	}

															// Return OK
	return true;
}

// Pip Test

/*
bool showRed = false;

unsigned int iterationDelta = getStreamManager().getCurrentIteration() - voxel->getLastStreamManagerIteration();

if(iterationDelta < 5)
{
	showRed = true;
}
*/

//showRed = voxel->getStreamLOD() > voxel->getCurrentLOD();

//showRed = (voxel->flag( pcloud::Visible ));

/*
if(voxel->getNumRequestLODs() >= 1)
{
	showRed = true;
}

if(showRed)
{
	unsigned int r = voxel->getNumRequestLODs() * 50;
	if(r > 255)
		r = 255;
	unsigned char notVisible[] = {r, 0, 0};
	voxel->setVoxelRGB(notVisible);
}
else
{
	unsigned char visible[] = {0, 255, 0};
	voxel->setVoxelRGB(visible);
}
*/

//visible &= !(voxel->flag( pcloud::WholeHidden ));

/*
if(visible == false)
{
	continue;
}
*/


ptds::DataSourcePtr StreamManager::getVoxelDataSource(Voxel *voxel, GlobalPagerData &globalPagerData, int userThread)
{
	return getDataSourceManager().getVoxelDataSource(voxel, userThread);
}


void StreamManager::setCurrentIteration(Iteration initIteration)
{
	iteration = initIteration;
}


StreamManager::Iteration StreamManager::getCurrentIteration(void)
{
	return iteration;
}


void StreamManager::incrementCurrentIteration(void)
{
	setCurrentIteration(getCurrentIteration() + 1);
}


bool StreamManager::addStreamedVoxel(DataSourcePtr dataSource, Voxel *voxel)
{
	StreamScheduler		*	streamScheduler;

	if(streamScheduler = getStreamScheduler())
	{
		streamScheduler->addActiveDataSourceVoxel(dataSource, voxel);
		return true;
	}

	return false;
}


bool StreamManager::raceLimitStreaming(unsigned int minTimeMilliseconds) 
{
	pt::TimeStamp t;

	if(testLastRefreshInit == false)
	{
		testLastRefresh.tick();
		testLastRefreshInit = true;
															// Return OK to run streaming
		return true;
	}

	t.tick();

    uint lastRefresh = static_cast<uint>(testLastRefresh.delta_ms(testLastRefresh, t));

	if(lastRefresh < minTimeMilliseconds)
		return false;

	testLastRefresh = t;

	return true;
}


void StreamManager::setStreamScheduler(StreamScheduler *scheduler)
{
	streamScheduler = scheduler;
}


StreamScheduler *StreamManager::getOrCreateStreamScheduler(void)
{
	MutexScope mutexScope(streamManagerMutex);

	if(getStreamScheduler() == NULL)
	{
		setStreamScheduler(new StreamSchedulerSequential());
	}

	return getStreamScheduler();
}


StreamScheduler	*StreamManager::getStreamScheduler(void)
{
	return streamScheduler;
}


StreamDataSource *StreamManager::addReadVoxel(Voxel *voxel, DataSource *dataSource, bool *streamDataSourceCreated)
{
	StreamScheduler	 *streamScheduler;

	if(dataSource)
	{
		if(streamScheduler = getStreamScheduler())
		{
			return streamScheduler->addActiveDataSourceVoxel(dataSource, voxel, streamDataSourceCreated);
		}
	}

	return NULL;
}


void StreamManager::endReadSets(void)
{
	StreamScheduler *streamScheduler;

	if(streamScheduler = getStreamScheduler())
	{
		return streamScheduler->endStreamHostStreaming();
	}
}


bool StreamManager::processStreamHostsRead(void)
{
	StreamScheduler *streamScheduler;

	if(streamScheduler = getStreamScheduler())
	{
		return streamScheduler->processStreamHostsRead();
	}

	return false;
}


bool StreamManager::lockStreamManager(void)
{
	return streamManagerMutex.wait(PTRMI_STREAM_MANAGER_MUTEX_TIMEOUT);
}


bool StreamManager::spinLockStreamManager(void)
{
	return streamManagerMutex.wait(0);
}


bool StreamManager::releaseStreamManager(void)
{
	if(streamManagerMutex.release())
	{
		return true;
	}

	Status status(Status::Status_Error_Failed_To_Release_Stream_Manager);

	return false;
}


void StreamManager::setBeginEnd(bool inBeginEnd)
{
	beginEnd = inBeginEnd;
}


bool StreamManager::getBeginEnd(void)
{
	return beginEnd;
}


bool StreamManager::begin(void)
{
															// Lock StreamManager for access by this thread
	if(lockStreamManager())
	{
															// If already locked and in in Begin/End phase
		if(getBeginEnd())
		{
															// Release the lock once
			releaseStreamManager();
															// Return
			return true;
		}
															// Not in Begin/End phase, so flag as in begin/end
		setBeginEnd(true);
															// Get or create the stream scheduler
		if(getOrCreateStreamScheduler())
		{
															// Begin scheduler streaming
			getStreamScheduler()->beginStreaming();
															// Return OK
			return true;
		}
	}
															// Set status as failed
	Status status(Status::Status_Error_Failed_To_Lock_Stream_Manager);

	Status::log(L"Stream Manager Lock Failure on Thread : ", BeThreadUtilities::GetCurrentThreadId());
															// Return failed
	return false;
}


bool StreamManager::end(void)
{
															// Make sure this thread has access to the StreamManager or return immediately if not
	if(spinLockStreamManager())
	{
															// If in begin/end phase
		if(getBeginEnd())
		{
			if(getStreamScheduler())
			{
				getStreamScheduler()->endStreaming();
			}
															// Set not in begin/end phase
			setBeginEnd(false);
															// Release one lock just acquired
			releaseStreamManager();
		}
															// Release the stream manager
		releaseStreamManager();
	}
															// Return failed
	return true;
}


} // End ptds namespace


