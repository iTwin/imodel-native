#include "PointoolsVortexAPIInternal.h"

#undef _VERBOSE

#ifdef _VERBOSE
#define COUT_TRACE
#endif

#include <ptcloud2/Voxel.h>

#include <ptfs/filepath.h>
#include <ptcloud2/pod.h>

#include <ptengine/pointspager.h>
#include <ptengine/pointsscene.h>
#include <ptengine/pointsfilter.h>

#include <pt/timestamp.h>
#include <ptengine/engine.h>
#include <ptcmdppe/cmdstate.h>

#include <pt/trace.h>

#include <ptengine/globalpagerdata.h>

#include <ptengine/StreamManager.h>

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <PTRMI/Manager.h>
#endif


#define REQUEST_RANGE 64

#define	POINTS_PAGER_SLEEP	250		// Sleep 0.25 seconds between running pager thread. This limits racing and gives more CPU time to main application threads


#include <pt/trace.h>

#define		PAGER_THREAD 0
#define		_MEMAVAL (int64_t)(mem.dwAvailVirtual - ((1.0-pp.capacity) * mem.dwTotalVirtual))

using namespace pointsengine;
using namespace pt;


GlobalPagerData pp;

GlobalPagerData &getGlobalPagerData(void)
{
	return pp;
}

//---------------------------------------------------------
float PointsPager::physicalRAMUseage() 
{ 
	return pp.capacity; 
}
//---------------------------------------------------------
void PointsPager::physicalRAMUseage(float use) 
{ 
	pp.capacity = use; 
}
//---------------------------------------------------------
// File Add or Removal observer
// Ensures that pager closes files and re-builds paging
// list when things change
//---------------------------------------------------------
struct PagerFileObserver : public FileObserver
{
	void sceneRemove( pcloud::Scene *sc )
	{
		sceneRemove( sc, true );
	}
	void sceneRemove( pcloud::Scene *sc, bool clearFile )
	{
		bool pausedState = pp.pointsPager->paused();

		pp.pointsPager->pause();	// pause paging thread - most probably already paused
		pp.voxlist.clear();			// clear the cache of leaf nodes, will be refilled by paging thread
									// but this is safest thing to do with minimal performance impact

		/* close file handle */ 
		for (size_t i=0; i<pp.files.size(); i++)
		{
			if(pp.files[i].scene == sc)
			{
				std::set<ptds::DataSourcePtr> handles;

				for (int t=0; t<4; t++)
				{
					if (pp.files[i].handle[t]) 
					{
						handles.insert(pp.files[i].handle[t]);
						pp.files[i].open[t] = false;
						pp.files[i].handle[t] = 0;
					}
				}

				if (clearFile) pp.files[i].clear();

				std::set<ptds::DataSourcePtr>::iterator it = handles.begin();
				while (it != handles.end())
				{
					try
					{
						ptds::dataSourceManager.close(*it);
					}
					catch(...){ /* invalid handle */ }
					++it;
				}
				break;
			}
		}
		if (!pausedState) pp.pointsPager->unpause();	
	}
	void sceneAdd( pcloud::Scene *Scene )
	{
		/* nothing to do */ 
	}
};
PagerFileObserver g_pagerFileObserver;
//---------------------------------------------------------
// constructor
//---------------------------------------------------------
PointsPager::PointsPager()
{
	pp.pager = 0;
	pp.pointsPager = this;
	_pager = NULL;
}
//---------------------------------------------------------
// destructor 
//---------------------------------------------------------
PointsPager::~PointsPager()
{
	pp.quit = true;

#ifndef POINTOOLS_API_INCLUDE
	if (pp.stateicon)
		delete pp.stateicon;
#endif

	try 
	{
		delete pp.pager;
	}
	catch(...){}

	pp.pager = 0;
}
//---------------------------------------------------------
// initialize Points Pager
//---------------------------------------------------------
bool PointsPager::initialize()
{
	PTTRACE("PointsPager::initialize"); 

	Pager p;
	pp.pager = new std::thread(p);
	pp.live = false;

	thePointsScene().addFileObserver( &g_pagerFileObserver );
	return true;
}
//---------------------------------------------------------
//
//---------------------------------------------------------
void PointsPager::purge()
{
	if (pp.pointsPager->paused())
		((PointsPager::Pager*)pp.pointsPager->_pager)->purgeData();
	else pp.purge = true;
}
//---------------------------------------------------------
// Pager destructor
//---------------------------------------------------------
PointsPager::Pager::~Pager()
{
	PTTRACE("PointsPager::~Pager"); 

	pp.complete = false;
	pp.pager = 0;

	if (!pp.quit && pp.working)
	{
#ifndef POINTOOLS_API_INCLUDE
		if (pp.stateicon)
			pp.stateicon->state(2);
#endif
        for (size_t i = 0; i < pp.files.size(); i++)
		{
			if (pp.files[i].open[_thread] 
				&& pp.files[i].handle[_thread]->validHandle())
			{
				try
				{
					ptds::dataSourceManager.close(pp.files[i].handle[_thread]);	//will assert if handle invalid
				}
				catch(...){}

				pp.files[i].clear();
			}
		}
	}
	pp.working = false;
}
PointsPager::Pager::Pager() 
{ }
//---------------------------------------------------------
// PAGER | functor Operator
//---------------------------------------------------------

void PointsPager::Pager::operator ()()
{
	PTTRACE("PointsPager::operator()");

	if(pp.pointsPager)
	{
		pp.pointsPager->_pager = this;
	}
	else
	{
		Status::log(L"Error: PointsPager::Pager::operator() failed to initialize _pager", L"");
	}

	/*run the engine*/ 
	pp.run = true;
	pp.working = false;

	_buffer = new ubyte[LOAD_BLOCK_SIZE];
	_thread = PAGER_THREAD;

	while (pp.run)
	{
		if (!pp.pointsPager->paused())
		{
			processRequests(getStreamManager());
			pp.working = false;
			pp.complete = false;
		}

        BeThreadUtilities::BeSleep(POINTS_PAGER_SLEEP);
	};

}
//---------------------------------------------------------
// load point cloud Structure only
//---------------------------------------------------------
pcloud::Scene::CreateSceneResult PointsPager::openScene(pcloud::Scene *scene)
{
	PTTRACE("PointsPager::openScene"); 

	pcloud::PodJob job(scene, scene->filepath());

	bool success = pcloud::PodIO::openForRead(job);

	if (success)
	{
		if (!pcloud::PodIO::readVersion(job)) 
		{
			return pcloud::Scene::PODVersionNotHandled;
		}
		if (!pcloud::PodIO::readHeader(job))
		{
			return pcloud::Scene::InvalidPODFile;	// TODO: return codes
		}

		/*store this handle*/ 
		pp.files.push_back(iodata(scene, 0, job.filepath, false, PAGER_THREAD));

		for (uint i=0; i< scene->size(); i++)
		{
			pcloud::PointCloud *pc = scene->cloud(i);
			if (!pcloud::PodIO::readCloudStructure(job, pc))
			{
				return pcloud::Scene::InvalidPODFile;	// TODO: return codes
			}

            Voxel::FileIndex file = static_cast<Voxel::FileIndex>(pp.files.size() - 1);
			
			for (size_t j=0;j<	pc->voxels().size(); j++)
			{
				pc->voxels()[j]->fileIndex(file);
			}
		}
	}

	pcloud::PodIO::close(job);
	return pcloud::Scene::Success;
}
//---------------------------------------------------------
bool PointsPager::closeSceneFile(pcloud::Scene *scene)
{
	g_pagerFileObserver.sceneRemove(scene, false);
	return true;
}
//---------------------------------------------------------
bool PointsPager::reopenScene(pcloud::Scene *scene)
{
	// find the scene file
	bool pausedState = pp.pointsPager->paused();
	bool retval = false;

	pp.pointsPager->pause();	// pause paging thread - most probably already paused

	for (size_t i=0; i<pp.files.size(); i++)
	{
		if(pp.files[i].scene == scene)
		{
			// unblock for open - pager thread will pick up and open file
			for (int t=0; t<4; t++)
			{
				pp.files[i].blocked[t] = false;
			}
			retval = true;
		}
	}
	if (!pausedState) pp.pointsPager->unpause();	
	return retval;
}
//---------------------------------------------------------
inline static int voxzorder(const pcloud::Voxel *a, const pcloud::Voxel *b)
{	
	return a->priority() > b->priority() ? 1 : 0;
}
//---------------------------------------------------------
inline static int voxuseorder(const pcloud::Voxel *a, const pcloud::Voxel *b)
{	
	return a->priority() < b->priority() ? 1 : 0;
}
//---------------------------------------------------------
// Balance data from voxels
//---------------------------------------------------------
void PointsPager::Pager::balanceMemoryLoad( int deltamb )
{
	PointsScene::VoxIterator b = pp.voxlist.begin();
	PointsScene::VoxIterator e = pp.voxlist.end();
	PointsScene::VoxIterator i;

	pcloud::Voxel *vox =0;
	//uint bytes, offset;
	//int pnts;
#ifdef _VERBOSE
	std::cout << "adjusting " << deltamb << "mb ";
#endif
	if (deltamb < 0)
	{
		int64_t loose_bytes = -deltamb * 1024 * 1024;
		int64_t bytespurged = 0;//purgeData( -deltamb );

		if (bytespurged >= loose_bytes) return;
		else
		{
			loose_bytes -= bytespurged;

			for (int j=0; j<2; j++)
			{
				/* more reduction required, make some voxels out of core */ 
				i = e;
				while (i!=b)
				{
					--i;
					vox = *i;

					if ( !vox->flag( pcloud::Visible ) )
					{
						for (int c=1; c<MAX_LOAD_CHANNELS; c++)
						{
							pcloud::DataChannel *dc = const_cast<pcloud::DataChannel*>(vox->channel(c));
							if (!dc || !dc->size() ) continue;						
							loose_bytes -= dc->bytesize();
							dc->dump();
						}						
					}
					/* find voxels that are 64kb or larger */ 
					if (j || vox->lodPointCount() > 32768)
					{					

                        std::unique_lock<std::mutex> vlock(vox->mutex());
						if (vlock.try_lock())
						{
							vox->flag( pcloud::OutOfCore, true );

							for (int c=1; c<MAX_LOAD_CHANNELS; c++)
							{
								pcloud::DataChannel *dc = const_cast<pcloud::DataChannel*>(vox->channel(c));
								if (!dc || !dc->size() ) continue;						
								loose_bytes -= dc->bytesize();
								dc->dump();
							}
							if (loose_bytes <= 0) break;
						}
					}
				}
				if (loose_bytes <= 0) break;
			}
		}
#ifdef _VERBOSE
		std::cout << "(" << loose_bytes << " not lost" << std::endl;
#endif
	}
	else
	{
		int64_t gain_bytes = deltamb * 1024 * 1024;

		i = e;
		while (i!=b)
		{
			--i;
			vox = *i;
			
			if (vox->flag( pcloud::OutOfCore ))
			{
                std::unique_lock<std::mutex> vlock(vox->mutex(), std::try_to_lock);
				if (!vlock.owns_lock()) continue;

				vox->flag( pcloud::OutOfCore, false );

				for (int c=1; c<MAX_LOAD_CHANNELS; c++)
				{
					pcloud::DataChannel *dc = const_cast<pcloud::DataChannel*>(vox->channel(c));
					if (!dc || !dc->size() ) continue;						
					gain_bytes -= static_cast<int64_t>((dc->multiple() * dc->typesize()) * vox->getCurrentLOD() * vox->fullPointCount());
				}
				if (gain_bytes <= 0) break;
			}
		};
	}
}
//---------------------------------------------------------
// Purge data from voxels
//---------------------------------------------------------
int64_t PointsPager::Pager::purgeData( int mb )
{
	PointsScene::VoxIterator b = pp.voxlist.begin();
	PointsScene::VoxIterator e = pp.voxlist.end();
	PointsScene::VoxIterator i;

	int64_t bytes2loose = mb * 1024 *1024;

	pcloud::Voxel *vox =0;
	uint bytes, offset;
	int pnts;

	i = e;
	while (i!=b)
	{
		///if (pp.pointsPager->paused()) return;
		--i;
		vox = *i;

        std::unique_lock<std::mutex> vlock(vox->mutex(), std::try_to_lock);

		if (vlock.owns_lock())
		{
			float amount=1;
			if (vox->flag(pcloud::WholeClipped)) amount = 0;
			if (!vox->flag(pcloud::Visible)) amount = 0;

			for (int c=1; c<MAX_LOAD_CHANNELS; c++)
			{
				pcloud::DataChannel *dc = const_cast<pcloud::DataChannel*>(vox->channel(c));
				if (!dc || c == pcloud::PCloud_Grid || !dc->size() ) continue;
				int currsize = dc->bytesize();

				if (amount == 0)
				{
					vox->resizeChannelToRequestedLod((pcloud::Channel)c, 0, pnts, bytes, offset);
					bytes2loose -= currsize;
				}
				else
				{
					float current = (float)dc->size() / vox->fullPointCount();
					amount = vox->getCurrentLOD();
					
					bytes = 0;

					if (current > amount)
						vox->resizeChannelToRequestedLod((pcloud::Channel)c, amount, pnts, bytes, offset);
					bytes2loose -= (currsize - bytes);
				}
			}
			if (mb && bytes2loose <= 0) break;
		}
	};
	pp.purge = false;

	return (mb*1024*1024) - bytes2loose;
}
//---------------------------------------------------------
// memory Usage count
//---------------------------------------------------------
int64_t memoryUsage()
{
	PointsScene::UseSceneVoxels voxelslock(pp.voxlist, pp.voxlistState);
	if (!pp.voxlist.size()) return 0;

	PointsScene::VoxIterator b = pp.voxlist.begin();
	PointsScene::VoxIterator e = pp.voxlist.end();
	PointsScene::VoxIterator i;

	pcloud::Voxel *vox =0;
	//uint bytes, offset;
	//int pnts;

	int64_t bytesUsed = 0;

	i = b;
	while (i!=e)
	{
		vox = *i;
	
		for (int c=1; c<MAX_LOAD_CHANNELS; c++)
		{
			if (vox->channel(c))
			{
				bytesUsed += vox->channel(c)->bytesize();
			}
		}
		++i;
	};

	return bytesUsed;// / (1024 * 1024);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int PointsPager::pagingIteration() const 
{ 
	return pp.iteration; 
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
uint PointsPager::KBytesLoaded(bool reset) 
{
	uint kb = static_cast<uint>(pp.bytesLoaded / 1024);
	if (reset)
		pp.bytesLoaded = 0;

	return kb;
}	
//-----------------------------------------------------------------------------
// num points loaded weighted by visibility - intended to judge viewport refresh 
// timing
//-----------------------------------------------------------------------------
uint PointsPager::pointsLoadedMetric(bool reset)
{
	uint pnts = static_cast<uint>(pp.weightedNumPntsLoaded);
	if (reset) pp.weightedNumPntsLoaded = 0;

	return pnts;
}
//-----------------------------------------------------------------------------
//static 
PointsPager::MemMode determineMemoryMode( int64_t &available )
{
	PointsPager::MemMode memoryMode = PointsPager::MemPlenty;
	float memCoef = (float)pp.memoryUsed / pp.memoryTarget;

	if (memCoef > 0.35) memoryMode = PointsPager::MemOK;
	if (memCoef > 0.55) memoryMode = PointsPager::MemPrudent;
	if (memCoef > 0.75) memoryMode = PointsPager::MemUrgent;
	if (memCoef > 0.98) memoryMode = PointsPager::MemCritical;

#if defined (BENTLEY_WIN32)  //NEEDS_WORK_VORTEX_DGNDB
	MEMORYSTATUS mem;
	GlobalMemoryStatus(&mem);
	uint memload = mem.dwMemoryLoad;
    UNUSED_VARIABLE(memload);
	available = _MEMAVAL;

	/* auto mem target management */ 
	if (!pp.useMemoryTarget)
	{
		int mb = static_cast<int>(available / (1024 * 1024));
		pp.memoryTarget = static_cast<int64_t>(available * pp.capacity + pp.memoryUsed);

		memoryMode = PointsPager::MemPlenty;

		if (mb <  50) memoryMode = PointsPager::MemCritical;
		else if (mb < 100) memoryMode = PointsPager::MemUrgent;
		else if (mb < 250) memoryMode = PointsPager::MemPrudent;
		else if (mb < 500 ) memoryMode = PointsPager::MemOK;
	}
#endif

	return memoryMode;
}
//-----------------------------------------------------------------------------
bool checkForRemoval()
{
	static bool had_data = false;
	if (!had_data && pp.voxlist.size()) had_data = true;
	else if (!pp.voxlist.size())
	{
		if (had_data)
		{
			/*check for open files, close if any	*/ 
			for (size_t f=0; f<pp.files.size(); f++)
			{
				//if (pp.files[f].open)
				//{
				//	pp.files[f].open[PAGER_THREAD] = false;
				//	if (ptds:IO::validHandle(pp.files[f].handle[PAGER_THREAD]))
				//		ptds:IO::close(pp.files[f].handle[PAGER_THREAD]);
				//}
			}
			had_data = false;
			pp.files.clear();
			return true;
		}
	}
	return false;;
}

//---------------------------------------------------------
// PAGER | Tuning Parameters
//---------------------------------------------------------
void PointsPager::setCacheSizeMb( int mb )
{
	pp.memoryTarget = mb * 1024 * 1024;
	pp.useMemoryTarget = true;
}
//-----------------------------------------------------------------------------
int PointsPager::getCacheSizeMb()
{
    return static_cast<int>(pp.memoryTarget / (1024 * 1024));
}
//-----------------------------------------------------------------------------
void PointsPager::useAutoCacheSize()
{
	pp.useMemoryTarget = false;
}
//---------------------------------------------------------
// PAGER | check for pause / quit
//---------------------------------------------------------
bool PointsPager::Pager::checkFlags()
{	
	if (pp.disableLoading
		|| !pp.run  
		|| pp.quit 
		|| pp.pointsPager->paused())
	{ 
		pp.working = false;
		return false;
	} 
	else if (pp.purge) purgeData();
	return true;
}
//---------------------------------------------------------
// PAGER | process Requests
//---------------------------------------------------------
void PointsPager::Pager::processRequests(pointsengine::StreamManager &streamManager)
{		
	PTTRACE("PointsPager::processRequests"); 

    std::unique_lock<std::mutex> pauseLock( pp.pausemutex );

	/* locals */ 
	//pcloud::Voxel	*vox;
	//int				lod, req;
	int64_t			memAval;

	/* timer */ 
	pt::TimeStamp	t0;

	t0.tick();		// start timer

	pp.working	= true;
	pp.live		= true;

	if (!checkFlags())										//check for pause or quit flags
	{
		return;
	}

// Pip Option
// PTRMI::getManager().pingInactiveHosts();

	Status::log(L"Stream Manager start processRequests() on thread : ", BeThreadUtilities::GetCurrentThreadId());

															// Begin Streaming. Clear all iteration based data structures.
	if(streamManager.begin() == false)
	{
		return;
	}

	/* update voxel list */ 
	PointsScene::UseSceneVoxels voxelslock(pp.voxlist, pp.voxlistState);
	if (!pp.voxlist.size())
	{
		pp.working = false;

		streamManager.end();

		Status::log(L"Stream Manager end processRequests() on thread : ", BeThreadUtilities::GetCurrentThreadId());

		return;
	}
	
	/* establish mode */ 
	MemMode memMode = determineMemoryMode(memAval);
															// Set up streaming parameters
	StreamManagerParameters	streamManagerParameters(streamManager, *this, pp.voxlist, memMode, pp);

															// Do Streaming
	streamManager.processRequestsStreamed(streamManagerParameters);

															// End Streaming
	streamManager.end();


	Status::log(L"Stream Manager end processRequests() on thread : ", BeThreadUtilities::GetCurrentThreadId());

#ifdef _VERBOSE
	std::cout << std::endl;
	switch (memMode)
	{
		case MemPlenty:
			std::cout << "Plenty:";
			break;

		case MemOK:
			std::cout << "OK:";
			break;

		case MemPrudent:
			std::cout << "Prudent:";
			break;

		case MemUrgent:
			std::cout << "Urgent:";
			break;

		case MemCritical:
			std::cout << "Critical:";
			break;
	}
	std::cout << " " << (int)(pp.memoryUsed / (1024 * 1024)) << "/" << (int)(pp.memoryTarget / (1024 *1024)) << " ";
#endif

#ifndef _DEBUG
	pp.voxlist.sort(memMode > MemPrudent ? voxuseorder : voxzorder);
#endif	


#ifdef XXX

	if (!pp.voxlist.size()) return;	// nothing to do

	/* SET UP ITERATION OVER LEAF LIST IN DEPTH ORDER	*/ 
	PointsScene::VoxIterator b = pp.voxlist.begin();
	PointsScene::VoxIterator e = pp.voxlist.end();
	PointsScene::VoxIterator i;

	for (i=b; i!=e; i++)
	{
		vox = *i;
			
		float	am = vox->getRequestLOD();		
		req		= REQUEST_RANGE * am;
		lod		= REQUEST_RANGE * vox->getCurrentLOD();	
		
		// if we have very low lod but its still more than the threshold number of points
		// important to do load

		if (am > 0 && !req && vox->fullPointCount() * am > LOW_LOD_LOAD_THRESHOLD)
		{
			req = 1;				// handle very low lod situation

			if (vox->getCurrentLOD() > 0 && !lod) 
				lod = 1;	
		}

		bool	ooc = vox->flag( pcloud::OutOfCore );	// this leaf is OOC
		bool	new_ooc = ooc;						
		bool	vis = vox->flag( pcloud::Visible );		// is visible

		/* load logic */ 
		switch (memMode)
		{
		case MemPlenty:
			if (ooc) new_ooc = false;
			if ( req < lod ) am = -1; /* prevent unload */ 
			break;

		case MemOK:
			if (ooc && req > 0.2f * REQUEST_RANGE)
				new_ooc = false;
			else if ( req < lod ) am = -1; /* prevent unload */ 
			break;

		case MemPrudent:

			if ( !vis && req > 1) am = 0;
			else if (ooc && req > 0.75f * REQUEST_RANGE)
				new_ooc = false;			
			else if (req == lod) continue;
			break;

		case MemUrgent:
			if ( !vis && req > 1) 
				am = 0;
			else if ( !ooc || lod > 0.5 * REQUEST_RANGE )
			{
				new_ooc = true;
				am = 0;//0.5;
			}
			/* CHECK LOCKing */ 
			else if (req == lod) continue; 
			break;

		case MemCritical:
			if ( !ooc || lod > 0 )
			{
				new_ooc = true;
				am = 0;
			}
			else continue;
		}
		int ami = am * REQUEST_RANGE;
		if (am && ami == 0) ami = 1;

		/* unload */ 
		if ((am >= 0 && ami < lod) || (!ooc && new_ooc)) 
		{
            std::unique_lock<std::mutex> vlock( vox->mutex(), std::try_to_lock );
			if (vlock.owns_lock())
			{
				vox->flag( pcloud::OutOfCore, new_ooc );
				if (am >= 0)
					pp.memoryUsed -= VoxelLoader::unloadVoxel( vox, am, false );
#ifdef _VERBOSE
				std::cout << "-";
#endif
			}
		}
		/* load */ 
		else if (ami > lod || (ooc && !new_ooc))
		{
			if (!checkFlags()) return;	//check for pause or quit flags

            std::unique_lock<std::mutex> vlock( vox->mutex(), std::try_to_lock );
			if (vlock.owns_lock())
			{
				vox->flag( pcloud::OutOfCore, new_ooc );
				vox->setRequestLOD( am );

				req = REQUEST_RANGE * vox->getRequestLOD();
				lod = REQUEST_RANGE * vox->getCurrentLOD();

				if (req < 1) req = 1;	// low lod handling

				if (req > lod)
				{
					int rb = vox->requestBytes();
					uint bytesLoaded = loadVoxel( vox, false );
#ifdef _VERBOSE
					std::cout << "+";
#endif
					if (bytesLoaded >= 0) pp.memoryUsed += bytesLoaded;
					else /* memory failure */ 
					{
						purgeData(rb*2);
						bytesLoaded = loadVoxel( vox, false );
						if (bytesLoaded > 0) pp.memoryUsed += bytesLoaded;
					}
				}
			}
		}

		pt::TimeStamp t1;
		t1.tick(); if (pt::TimeStamp::delta_ms(t0,t1) > 2000) break;

		if (!checkFlags()) return;	//check for pause or quit flags

		if (pp.iteration % 30 == 0) 
			pp.memoryUsed = memoryUsage();

		if (!pp.voxlist.size()) return;

		memMode = determineMemoryMode(memAval);
		++pp.iteration;
	
	}

#endif

}
//---------------------------------------------------------
// VOXELLOADER | Constructor
//---------------------------------------------------------
VoxelLoader::VoxelLoader(pcloud::Voxel *vox, float amount, bool pause, bool lock, bool dump, int thread, bool skipLoad)
{
	_buffer = 0;
	if (vox && (amount > vox->getCurrentLOD() || skipLoad))
	{
		_voxel = vox;
		_lock = lock;

		_skipLoad = skipLoad;

		_paused = pp.pointsPager->paused();

		if (!_paused && pause)
		{
			pp.pointsPager->pause();
		}

		/*store current lod - not the current request*/ 
		_lod = _voxel->getCurrentLOD();
		_thread = thread;

		// Pip Option - This might not be removable
		//		vox->setRequestLOD(amount);

		if(skipLoad == false)
		{
			loadVoxel(_voxel, amount, false, lock);
		}

		setDump(dump);
	}
	else
	{
		setVoxel(NULL);
		setDump(false);
	}
}
VoxelLoader::~VoxelLoader()
{
	if (_voxel)
	{
		if (_dump)
		{
			_voxel->clipRequestLODMax(_lod);

			unloadVoxel(_voxel, _lod, _lock);
		}
		if (!_paused)		pp.pointsPager->unpause();
	}
};
//---------------------------------------------------------
// VOXELLOADER | LoadSample
// loads a single point as a sample value
//---------------------------------------------------------
bool VoxelLoader::loadSample(pcloud::Voxel *vox)
{
	/* check pod version */ 
	//if (sample is stored and already retreived)
	//{
	//	return true;
	//}
	//else
	{
		return false;
	}
}
//---------------------------------------------------------
// Open Voxel File
//---------------------------------------------------------
ptds::DataSourcePtr VoxelLoader::openFile(pcloud::Voxel *vox, int thread)
{
															/*get the file index*/ 
	Voxel::FileIndex file = vox->fileIndex();

															/*check for null file index*/ 
	if (file == Voxel::FILE_INDEX_NULL)
	{														/*open scene file*/ 
		return NULL;
	}

	DataSourcePtr dataSource = pp.files[file].handle[thread];

	if (!pp.files[file].open[thread] && !pp.files[file].blocked[thread])
	{
		dataSource = pp.files[file].handle[thread] = ptds::dataSourceManager.openForRead(&(pp.files[file].getFilePath()));

		if(dataSource == NULL)
		{
															// Unable to open new data source, so return NULL
			return NULL;
		}

		if(dataSource->validHandle())
		{
			pp.files[file].open[thread] = true;
		}
		else
		{
			return 0;
		}
	}

	return dataSource;
}
//---------------------------------------------------------
// VOXELLOADER | loadVoxel
//---------------------------------------------------------
int VoxelLoader::loadVoxel(pcloud::Voxel *vox, float lodRead, bool full, bool lock)
{

	/*load in extra data required*/ 
	Voxel::FileIndex file = vox->fileIndex();
	int bytes_loaded = 0;

	/*check for null file index*/ 
	if (file == Voxel::FILE_INDEX_NULL)
	{ 
		// open scene file
	}

	if(openFile(vox, _thread))
	{
		/*load data into channels*/ 
		uint bytes, offset, prev_channel_size;

		prev_channel_size = 0;
		
		uint chfilter = pointsFilteringState().displayChannelFilter();
		bool dofilter = pointsFilteringState().filterPagingByDisplay();
		uint channelbit = 1;

		std::unique_lock<std::mutex> vlock(vox->mutex(), std::defer_lock);

		try 
		{
			if (lock && !vlock.owns_lock())
			{
				vlock.lock(); 
			}
		}
		catch(...)
		{
			return 0;
		}

		int actual_points = 0;

		{
			float save_req = vox->getRequestLOD();
            UNUSED_VARIABLE(save_req);

			if (full)
			{
// Pip Option - This might not be removable
//				vox->setRequestLOD(LOD_MAX);

				lodRead = 1.0f;
			}

			for (int c=1; c<MAX_LOAD_CHANNELS; c++)
			{
				pcloud::DataChannel *dc = const_cast<pcloud::DataChannel*>(vox->channel(c));

				if (!dc || c == pcloud::PCloud_Grid)
					continue;

				pcloud::Channel ch = pcloud::channel(c);
                UNUSED_VARIABLE(ch);
				
				/*reallocate voxel data channels*/ 
				int64_t pos = vox->filePointer() + prev_channel_size;
				
				/*this is the size of full channel data on disk*/ 
                prev_channel_size += static_cast<uint>(vox->fullPointCount() * dc->typesize() * dc->multiple());

				/*can't do this earlier because we need offset value*/ 
				if (dofilter && chfilter & channelbit)
				{
					channelbit <<= 1;
					continue;
				}
				channelbit <<= 1;
				
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

// Pip Option
//				if(vox->getRequestLOD() > LOD_ZERO)
				if(lodRead > LOD_ZERO)
				{
															// Enforce reading of lodRead rather than lodRequest
//					float lodAmount = lodRead / vox->getRequestLOD();

//					if (vox->resizeAndFillChannelToRequestedLod(pcloud::channel(c), lodAmount, actual_points, bytes, offset, pp.files[file].handle[_thread], pos, _buffer))
					if (vox->resizeAndFillChannelToRequestedLod(pcloud::channel(c), lodRead, actual_points, bytes, offset, pp.files[file].handle[_thread], pos, _buffer))
					{
						bytes_loaded += bytes;
					}
					else
					{
						return 0;
					}
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
// Pip Option - This might not be removable
//					vox->setRequestLOD(save_req);
				}
			}
		}
	}

	return bytes_loaded;
}
//---------------------------------------------------------
// VOXELLOADER | unloadVoxel
//---------------------------------------------------------
int VoxelLoader::unloadVoxel(pcloud::Voxel *vox, float amount, bool lock )
{
	uint bytesUnloaded = 0;

// Pip Option - This might not be removable
//	vox->setRequestLOD( amount );

	if(vox == NULL)
	{
		return 0;
	}
	
    std::unique_lock<std::mutex> vlock(vox->mutex(), std::defer_lock);
	try { if (lock) vlock.lock(); }
	catch(...) { }

															// If currently resized to receive stream data, do not unload
	if(vox->getResizedToStream())
	{
		return 0;
	}
															// Do not unload if current LOD is same as LOD to unload to
	if(vox->compareLOD(amount, vox->getCurrentLOD()) == Voxel::LODEqual)
	{
		return 0;
	}

	for (int c=0; c<MAX_LOAD_CHANNELS; c++)
	{
		pcloud::DataChannel *dc = const_cast<pcloud::DataChannel*>(vox->channel(c));
		if (!dc) continue;
		
		int points;
		uint bytes, offset;
		
		int prevbytes = dc->bytesize();
		vox->resizeChannelToRequestedLod(pcloud::channel(c), amount, points, bytes, offset, false);

		bytesUnloaded += prevbytes - dc->bytesize();
	}
	return bytesUnloaded;
}
//---------------------------------------------------------
// PAGER | loadVoxel
//---------------------------------------------------------
int PointsPager::Pager::loadVoxel(pcloud::Voxel *vox, float lodRead, bool lock)
{
	Voxel::LOD lod = vox->getCurrentLOD();

	// weighted load metric

	int64_t numPoints = vox->getNumPointsAtLOD(lodRead) - vox->getNumPointsAtLOD(lod) ;

//	uint numPoints = (lodRead - lod) * vox->fullPointCount();

	
	if (vox->flag(pcloud::WholeClipped)	||
		vox->flag(pcloud::WholeHidden)	||
		vox->flag(pcloud::OutOfCore)	||
		numPoints < 0)
	{
		numPoints = 0;
	}

	if (vox->flag(pcloud::PartClipped))
		numPoints *= 0;

// Pip Option: Results in zero !! Replace with something better ?
//	numPoints *= (lodRead > 1.0f ? 1.0f : lodRead);

	pp.weightedNumPntsLoaded += numPoints;	

	// bytes loaded
	int bytesLoaded =  VoxelLoader::loadVoxel(vox, lodRead, false, lock);
	pp.bytesLoaded += bytesLoaded;		

	return bytesLoaded;
}
//---------------------------------------------------------
// PAGER | unloadVoxel
//---------------------------------------------------------
void PointsPager::Pager::unloadVoxel(pcloud::Voxel *vox, bool lock)
{
	VoxelLoader::unloadVoxel(vox, 0, lock);
}
//---------------------------------------------------------
// pager access
//---------------------------------------------------------
void PointsPager::stop()				
{ 
	pp.run = false; 
	while (pp.working) BeThreadUtilities::BeSleep(10);
	pp.voxlist.clear(); 
}
//---------------------------------------------------------
void PointsPager::completeRequests()	
{ 
	pp.complete = true; 
	while (pp.complete) 
        BeThreadUtilities::BeSleep(10);
}
//---------------------------------------------------------
bool PointsPager::pause()				
{ 
	// if its paused still need to check is lock was acquired
	if (_paused && !pp.pausemutex.try_lock()) 
		return true;

	if (!_paused) 
		pp.pausemutex.lock(); 
	
	pp.voxlist.clear(); 
	_paused = true; 

	return true; 
}
//---------------------------------------------------------
bool PointsPager::softPause()				
{ 
	if (_paused)
		return true;

	_paused = true; 
	pp.pausemutex.try_lock(); 
	
	//pp.voxlist.clear(); 
	return false; 
}
//---------------------------------------------------------
bool PointsPager::isPaused() const
{
	return _paused;
}
//---------------------------------------------------------
bool PointsPager::unpause()				
{ 
	if (_paused)
	{
		_paused = false;
		try
		{
			pp.pausemutex.unlock(); 
		}
		catch(...)
		{
			// mutex already unlocked, no big deal
		}
		return true; 
	}
	return true;
}

//---------------------------------------------------------
// PAGER | operator = (needed for boost::thread)
//---------------------------------------------------------
PointsPager::Pager &PointsPager::Pager::operator = (const PointsPager::Pager &p)
{
	return *this;
}
#undef TRACER
