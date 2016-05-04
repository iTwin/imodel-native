#pragma once


#include <ptapi/PointoolsVortexAPI.h>

#include <ptcloud2/scene.h>
#include <ptengine/pointspager.h>
#include <ptengine/pointsScene.h>

#include <PTRMI/URL.h>
#include <mutex>

namespace pointsengine
{

//-----------------------------------------------------------------------------
// IO Data | Stores File paths and handles for each thread
//-----------------------------------------------------------------------------
class iodata
{

public:

	ptds::DataSourcePtr	handle[4];
	bool				open[4];
	bool				blocked[4];
	pcloud::Scene *		scene;

protected:

	ptds::FilePath		path;
	bool				fileRemote;

public:

	iodata(pcloud::Scene *s, const ptds::DataSourcePtr &h, const ptds::FilePath &p, bool o, int thread)
	{
		for (int i=0; i<4; i++)
		{
			handle[i]	= 0;
			open[i]		= false;
			blocked[i]  = false;
		}

		setFilePath(p);

		scene			= s;
		handle[thread]	= h;
		open[thread]	= o;
	}

	void clear(void)
	{
		for (int i=0; i<4; i++)
		{
			handle[i]	= 0;
			open[i]		= false;
		}

		scene = NULL;

		setFilePath(ptds::FilePath(L""));
		setFileRemote(false);
	}

	void operator = (const iodata &d) 
	{ 
		for (int i=0; i<4; i++)
		{
			handle[i]	= d.handle[i];
			open[i]		= d.open[i];
		}

		setFilePath(d.getFilePath());

		scene	= d.scene;
	}

	void setFilePath(const ptds::FilePath &p)
	{
		path = p;

		PTRMI::URL url = p.path();

		setFileRemote(url.isProtocolNetworked());
	}

	const ptds::FilePath &getFilePath(void) const
	{
		return path;
	}

	void setFileRemote(bool remote)
	{
		fileRemote = remote;
	}

	bool getFileRemote(void)
	{
		return fileRemote;
	}
};

//-----------------------------------------------------------------------------
// Global pager data
//-----------------------------------------------------------------------------

struct GlobalPagerData
{
	PointsPager *			pointsPager;
	std::thread*			pager;

	int64_t					bytesLoaded;
	int64_t					weightedNumPntsLoaded;

	/* working flags */ 
	bool					run;
	volatile bool			working;
	volatile bool			live;
	volatile bool			quit;  
	volatile bool			complete;
	volatile bool			disableLoading;
	volatile bool			purge;

	volatile int			iteration;

	/* tuning */ 
	float					capacity;
	int64_t					memoryTarget;
	int64_t					memoryUsed;
	bool					useMemoryTarget;
	bool					allowReduce;

	std::vector<iodata>		files;

	PointsScene::VOXELSLIST voxlist;
	int						voxlistState;

	std::mutex			pausemutex;


	GlobalPagerData()
	{
		/* thread object */
		pointsPager = 0;
		pager = 0;

		bytesLoaded = 0;

		/* working flags */ 
		run = false;
		working = false;
		live = true;
		quit = false;  
		complete = false;
		disableLoading = false;
		purge = false;

		int iteration = 0;

		/* tuning parameters*/ 
		capacity = 0.95f;
		memoryTarget = 1024 * 1024 * 1024;
		memoryUsed = 0;
		useMemoryTarget = false;
		allowReduce = true;

		weightedNumPntsLoaded = 0;

		voxlistState = -1;
	}
};

}
