/*----------------------------------------------------------*/ 
/* PointsPager.h											*/ 
/* Point Pager Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2007-2011						*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_ENGINE_POINTS_LOADER
#define POINTOOLS_ENGINE_POINTS_LOADER 1

#include <ptcloud2/Voxel.h>
#include <ptds/FilePath.h>
#include <ptengine/ptengine_api.h>
#include <set>

namespace pointsengine
{
	//---------------------------------------------------------
	// Voxel Loader for loading voxel channels
	//---------------------------------------------------------
	// Note thread = 0 for pager use, = 1 for main thread use
	//
	class PTENGINE_API VoxelLoader
	{
	public:
		VoxelLoader(pcloud::Voxel *vox=0, float amount = 0, bool pause=true, bool lock=true, bool dump = true, int thread = 1,  bool skipLoad = false);
		virtual ~VoxelLoader();

		static ptds::DataSourcePtr openFile			(pcloud::Voxel *vox, int thread=1);

		int							loadVoxel		(pcloud::Voxel* voxel, float lodRead, bool full, bool lock);
		int							unloadVoxel		(pcloud::Voxel* voxel, float amount, bool lock);
		bool						loadSample		(pcloud::Voxel *vox);

		int							getThread		(void)						{return _thread;}

		void						setPreviousLOD	(pcloud::Voxel::LOD lod)	{_lod = lod;}
		pcloud::Voxel::LOD			getPreviousLOD	(void)						{return _lod;}

		void						setVoxel		(pcloud::Voxel *voxel)		{_voxel = voxel;}
		pcloud::Voxel *				getVoxel		(void)						{return _voxel;}

		void						setDump			(bool dump)					{_dump = dump;}
		bool						getDump			(void)						{return _dump;}

		void						setLock			(bool lock)					{_lock = lock;}
		bool						getLock			(void)						{return _lock;}

	protected:

		void			*_buffer;
		int				_thread;

	private:
		pcloud::Voxel	*_voxel;
		float			_lod;
		bool			_lock;
		volatile bool	_paused; // Accessed from different threads so make sure it is read each time, not cached
		bool			_filterpaused;
		bool			_res;
		bool			_dump;
		bool			_skipLoad;
	};
}
#endif