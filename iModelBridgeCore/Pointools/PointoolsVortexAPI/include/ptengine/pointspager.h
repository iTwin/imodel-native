/*----------------------------------------------------------*/ 
/* PointsPager.h											*/ 
/* Point Pager Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_ENGINE_POINTS_PAGER
#define POINTOOLS_ENGINE_POINTS_PAGER 1

#include <ptcloud2/Voxel.h>
#include <ptfs/filepath.h>
#include <ptengine/ptengine_api.h>
#include <ptengine/module.h>
#include <ptengine/podCache.h>
#include <ptengine/voxelLoader.h>
#include <set>

#define	LOW_LOD_LOAD_THRESHOLD	512
#define	LOAD_BLOCK_SIZE 65536


namespace pointsengine
{
	class StreamManager;

	struct GlobalPagerData;
}

pointsengine::GlobalPagerData &getGlobalPagerData(void);


namespace pointsengine
{
	
class PTENGINE_API PointsPager : public Module
{

public:

	enum MemMode
	{
		MemPlenty =0,
		MemOK =1,
		MemPrudent=2,
		MemUrgent=3,
		MemCritical=4
	};


public:

	PointsPager();
	~PointsPager();

	bool initialize();
	bool pause();
	bool softPause();
	bool unpause();
	bool isPaused() const;

	pcloud::Scene::CreateSceneResult	openScene(pcloud::Scene *scene);
	
	// close a scene file but do not remove from cache
	// usually done to enable write to file for metadata update
	bool								closeSceneFile(pcloud::Scene *scene);

	// reopen a scene file and continue paging its data
	bool								reopenScene(pcloud::Scene *scene);

	static void		stop();
	static void		purge();
	static void		completeRequests();

	static uint		KBytesLoaded( bool reset = true );

	static uint		pointsLoadedMetric( bool reset = true );

	static void		physicalRAMUseage(float use);
	static float	physicalRAMUseage();

	static void		setCacheSizeMb( int mb );
	static int		getCacheSizeMb();
	static void		useAutoCacheSize();

	int	pagingIteration() const;

public:

	// PODCacheManager	m_podCache;	// not ready for release yet

	class Pager : public VoxelLoader
	{
	public:

					Pager				(void);
				   ~Pager				(void);

		Pager	&	operator =			(const Pager &p);
		void		balanceMemoryLoad	(int deltamb);
		__int64 	purgeData			(int mb=0);
		void		processRequests		(pointsengine::StreamManager &streamManager);
		bool		checkFlags			(void);
		int			loadVoxel			(pcloud::Voxel*, float lodRead, bool lock=true);
		void		unloadVoxel			(pcloud::Voxel*, bool lock=true);
		void		operator ()			(void);
	};

	void *_pager;
};
}


__int64								memoryUsage				(void);
pointsengine::PointsPager::MemMode	determineMemoryMode		(__int64 &available);


#endif
