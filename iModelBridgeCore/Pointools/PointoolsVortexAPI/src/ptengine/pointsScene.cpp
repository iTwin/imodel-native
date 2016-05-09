#include "PointoolsVortexAPIInternal.h"


#include <pt/project.h>

#include <ptengine/pointsScene.h>
#include <ptengine/pointsPager.h>
#include <ptengine/engine.h>

#include <pt/timestamp.h>

#include <ptengine/VoxelLODRemoveVisitor.h>

#include <stdio.h>

#include <algorithm>

using namespace pointsengine;

#define LIST_REFRESH_ITERATIONS 5
#define PT_SCENE_MULTIPLIER 10000

namespace ptscene
{
	PointsScene::VOXELSLIST _voxels;
	PointsScene::VOXELSLIST _rendervoxels;
	std::vector<pcloud::Scene *> _scenes;

    //&&MM no shared_mutex in c++ 11 need to wait for c++ 17. Are we really loosing something?
    // most of the lock are not shared.
	std::mutex _listusemutex;

	int _listuse = 0;
	bool _sortrequest = false;
	int _listvalid = 0;
	int _depthlistvalid = 0;

	typedef std::list< FileObserver * > FILE_OBS_LIST;
};
using namespace ptscene;

/*----------------------------------------------------------*/ 
/* Points Scene Voxel use object							*/ 
/*----------------------------------------------------------*/ 
inline static int voxsort(const pcloud::Voxel *a, const pcloud::Voxel *b)
{	
	return a->priority() > b->priority() ? 1 : 0;
}
//-----------------------------------------------------------------------------
PointsScene::UseSceneVoxels::UseSceneVoxels(VOXELSLIST &vlist, int &state) 
: _vlist(vlist)
{ 
	hasLock = false;
	if (!_scenes.size())
	{
		_vlist.clear();		
		vlist.clear();
		return;
	}
	int iteration = thePointsScene().getIteration();
	if (state != iteration && vlist.size())
	{
		_vlist.clear();		
		vlist.clear();
	}
	state = iteration;

	if (_vlist.size() != _voxels.size() || _listvalid<0)
	{
		while (_vlist.size() > _voxels.size()) _vlist.pop_back();
		while (_vlist.size() < _voxels.size()) _vlist.push_back(0);

		try 
		{
			hasLock = true;//_listusemutex.try_lock_shared();
		}
		catch(...) { hasLock = false; }
		if (!hasLock) return;
		/*copy the list*/ 

		VOXELSLIST::const_iterator it = _voxels.begin();
		VOXELSLIST::iterator itc = _vlist.begin();

		while (it != _voxels.end())
		{
			(*itc) = (*it);
			++it;
			++itc;
		}
		_listvalid++;
	}
	//
	//_available = lock.locked();
	//if (_available)	++_listuse;	
}
//-----------------------------------------------------------------------------
PointsScene::UseDepthSortedVoxels::UseDepthSortedVoxels(VOXELSLIST &vlist, int &state) 
: _vlist(vlist)
{ 
	hasLock = false;

	if (!_scenes.size())
	{
		_vlist.clear();
		vlist.clear();
		return;
	}

	int iteration = thePointsScene().getIteration();
	if (state != iteration)
	{
		_vlist.clear();		
		vlist.clear();
	}
	state = iteration;
	static int viteration= -4785;
	try 
	{
        //&&&M _listusemutex.try_lock_shared()
		hasLock = _listusemutex.try_lock();
	}
	catch(...) { hasLock = false; }
	if (!hasLock) return;

	if (_vlist.size() != _voxels.size() || _depthlistvalid<0)
	{
		/*copy the list*/ 
		while (_vlist.size() > _voxels.size()) _vlist.pop_back();
		while (_vlist.size() < _voxels.size()) _vlist.push_back(0);

		VOXELSLIST::const_iterator it = _voxels.begin();
		VOXELSLIST::iterator itc = _vlist.begin();

		while (it != _voxels.end())
		{
			(*itc) = (*it);
			++it;
			++itc;
		}
		_depthlistvalid ++;
	}
	/* sort if needed */ 
	if (theVisibilityEngine().getIteration() != viteration)
	{
		iteration = theVisibilityEngine().getIteration();
		_vlist.sort(voxsort); /* cause of issues, + not sure it helps performance */ 
	}
}
//-----------------------------------------------------------------------------
PointsScene::UseDepthSortedVoxels::~UseDepthSortedVoxels() 
{
    //&&MM shared_lock
	//if (hasLock) _listusemutex.unlock_shared();
    if (hasLock) _listusemutex.unlock();
}
//-----------------------------------------------------------------------------
PointsScene::UseSceneVoxels::~UseSceneVoxels()
{ 
	//if (hasLock) //_listusemutex.unlock_shared();
} 
/*----------------------------------------------------------*/ 
/* Points Scene												*/ 
/*----------------------------------------------------------*/ 
pcloud::Scene *PointsScene::operator [] (int i)
{
	return _scenes[i]; 
}
//-----------------------------------------------------------------------------
int PointsScene::size() 
{ 
	return static_cast<int>(_scenes.size());
}
//-----------------------------------------------------------------------------
PointsScene::VoxIterator PointsScene::voxbegin()
{ 
    std::lock_guard<std::mutex> lock(_mutex);
	return _voxels.begin(); 
}
//-----------------------------------------------------------------------------
PointsScene::VoxIterator PointsScene::voxend() 
{ 
    std::lock_guard<std::mutex> lock(_mutex);
	return _voxels.end(); 
}
//-----------------------------------------------------------------------------
const PointsScene::VOXELSLIST &PointsScene::voxels()
{ 
    std::lock_guard<std::mutex> lock(_mutex);
	return _voxels; 
}
//-----------------------------------------------------------------------------
PointsScene::PointsScene()
{
	_boundsDirty = true;	
	_iteration = 0;
}
//-----------------------------------------------------------------------------
PointsScene::~PointsScene()
{

}
//-----------------------------------------------------------------------------
bool PointsScene::initialize()
{
	return false;
}
//-----------------------------------------------------------------------------
void PointsScene::depthSortVoxels()
{
	//boost::try_mutex::scoped_try_lock lock(_listusemutex);

	//if (lock.locked() && !_listuse)	_voxels.sort(voxsort);
	//else							_sortrequest = true;
}
//------------------------------------------------------------
// adds a Scene that already has structure
//------------------------------------------------------------
void PointsScene::addScene(pcloud::Scene *sc)
{
	uint i;
	++_iteration;

	/* assign the handle to the scene and the point clouds within it*/ 
	static uint sceneIndex = 0;
	++sceneIndex;

	if(sc == NULL)
		return;

	sc->setKey(sceneIndex * PT_SCENE_MULTIPLIER);
	for (i=0; i<sc->size(); i++) sc->cloud(i)->setKey(sc->key() + i + 1);
	_sceneByKey.insert(std::pair<pt::ObjectKey, pcloud::Scene*>(sc->key(), sc));

    std::lock_guard<std::mutex> lock(_mutex);
	std::unique_lock<std::mutex> listlock(_listusemutex);

	for (i=0; i<_scenes.size(); i++) if (_scenes[i] == sc) return;

	int vcount = static_cast<int>(_voxels.size());
		
	for (i=0; i<sc->size(); i++)
	{
		pcloud::PointCloud *pc = (*sc)[i];

		int64_t guid = pc->guid();
		if (!guid)
		{
			pc->generateGuid();
		}
		_cloudByGuid.insert( std::pair<int64_t, pcloud::PointCloud*>( pc->guid(), pc) );

		if (pc->root())
		{
			for (int j=0; j<pc->voxels().size(); j++)
				_voxels.push_back(pc->voxels()[j]);
		}
	}
	// is this an instance?
	typedef std::map<pt::ObjectKey, pcloud::Scene*> SceneMap;
	SceneMap::iterator sci = _sceneByKey.begin();
	int instances = 0;
	pcloud::PointCloudGUID guid;
	
	if(sc->size() > 0 && sc->cloud(0))
	{
		guid = sc->cloud(0)->guid();
	}

	for (i=0; i<_scenes.size(); i++)
	{
		if(_scenes[i])
		{
			if(_scenes[i]->cloud(0))
			{
				if (_scenes[i]->cloud(0)->guid() == guid)
					++instances;
			}
		}
	}

	sc->setInstanceIndex( instances-1 );

	_boundsDirty = true;
	pt::BoundingBoxD bb;
	getBounds(bb);

	_scenes.push_back(sc);
	_listvalid = -LIST_REFRESH_ITERATIONS;
	_depthlistvalid = -LIST_REFRESH_ITERATIONS;
}
//-----------------------------------------------------------------------------
const pcloud::Scene * PointsScene::sceneBySceneOrCloudKey(const pt::ObjectKey &objk) const
{
	uint sckey = (objk / PT_SCENE_MULTIPLIER) * PT_SCENE_MULTIPLIER;
	std::map<pt::ObjectKey, pcloud::Scene*>::const_iterator it = _sceneByKey.find(sckey);
	return it == _sceneByKey.end() ? 0 : it->second;
}
//-----------------------------------------------------------------------------
pcloud::Scene * PointsScene::sceneBySceneOrCloudKey(const pt::ObjectKey &objk)
{
	uint sckey = (objk / PT_SCENE_MULTIPLIER) * PT_SCENE_MULTIPLIER;
	std::map<pt::ObjectKey, pcloud::Scene*>::iterator it = _sceneByKey.find(sckey);
	return it == _sceneByKey.end() ? 0 : it->second;
}
//-----------------------------------------------------------------------------
const pcloud::PointCloud * PointsScene::cloudByKey(const pt::ObjectKey &objk) const
{
	const pcloud::Scene *sc = sceneBySceneOrCloudKey(objk);

	if (sc)
	{
		uint cloudidx = objk % PT_SCENE_MULTIPLIER - 1;
		if (cloudidx < sc->size()) return sc->cloud(cloudidx);
	}
	return 0;
}
//-----------------------------------------------------------------------------
pcloud::PointCloud * PointsScene::cloudByKey(const pt::ObjectKey &objk)
{
	pcloud::Scene *sc = sceneBySceneOrCloudKey(objk);

	if (sc)
	{
		uint cloudidx = objk % PT_SCENE_MULTIPLIER - 1;
		if (cloudidx < sc->size()) return sc->cloud(cloudidx);
	}
	return 0;
}
//-----------------------------------------------------------------------------
const pcloud::PointCloud *PointsScene::cloudByGUID(const int64_t guid) const
{
	std::map<int64_t, pcloud::PointCloud*>::const_iterator it = _cloudByGuid.find(guid);
	return it == _cloudByGuid.end() ? 0 : it->second;
}
//-----------------------------------------------------------------------------
pcloud::PointCloud *PointsScene::cloudByGUID(const int64_t guid)
{
	std::map<int64_t, pcloud::PointCloud*>::iterator it = _cloudByGuid.find(guid);
	return it == _cloudByGuid.end() ? 0 : it->second;
}
//-----------------------------------------------------------------------------
void PointsScene::removeScene(pcloud::Scene *sc, bool del)
{
	++_iteration;

    std::lock_guard<std::mutex> lock(_mutex);
    std::unique_lock<std::mutex> listlock(_listusemutex);

	/* remove from maps */ 
	for (int c=0; c<sc->numObjects(); c++)
	{
		int64_t guid = sc->cloud(c)->guid();
		if (_cloudByGuid.find(guid) != _cloudByGuid.end())
			_cloudByGuid.erase( _cloudByGuid.find(guid) );
	};

	std::map<pt::ObjectKey, pcloud::Scene*>::iterator it = _sceneByKey.find(sc->key());
	if (it != _sceneByKey.end())
	{
		_sceneByKey.erase(it);
	}

	/* notify observers */ 
	FILE_OBS_LIST::iterator i = _fileObs.begin();
	while (i != _fileObs.end())
	{
		(*i)->sceneRemove( sc );
		++i;
	}

	typedef std::vector<pcloud::Scene*> SCENEVEC;
	
	for (SCENEVEC::iterator si=_scenes.begin(); si!=_scenes.end(); si++)
	{
		if (*si == sc)
		{
			_scenes.erase(si);
			break;
		}
	}

	/*to keep things at least linear, put the voxels into a map*/ 
	typedef std::map<const pcloud::Voxel*, VOXELSLIST::iterator> VOXMAP;
	VOXMAP voxmap;
	for (VOXELSLIST::iterator it = _voxels.begin(); it != _voxels.end(); it++)
		voxmap.insert(VOXMAP::value_type(*it, it));

	for (uint i=0; i<sc->size(); i++)
	{
		if ((*sc)[i]->root())
		{
			for (int j=0; j<(*sc)[i]->voxels().size(); j++)
			{
				VOXMAP::iterator V = voxmap.find((*sc)[i]->voxels()[j]);
				if (V!= voxmap.end()) _voxels.erase(V->second);
			}
		}
	}

	_boundsDirty = true;	
	if (del) delete sc;	
	_listvalid = -LIST_REFRESH_ITERATIONS;
	_depthlistvalid = -LIST_REFRESH_ITERATIONS;
}
//------------------------------------------------------------
// opens a pod scene file
//------------------------------------------------------------
pcloud::Scene *PointsScene::openScene(const ptds::FilePath &path, int &error)
{
	pcloud::Scene *scene = pcloud::Scene::createFromFile(path);

	if(true)
	{
		error = thePointsPager().openScene(scene);

		if (error == pcloud::Scene::Success)
		{
			addScene(scene);			
			error = pcloud::Scene::Success;
		
			/* notify observers */ 
			FILE_OBS_LIST::iterator i = _fileObs.begin();
			while (i != _fileObs.end())
			{
				(*i)->sceneAdd( scene );
				++i;
			}
		}
		else
		{
			delete scene;
			scene = 0;
		}		
	}
	else
	{
		delete scene;
		scene = 0;
		error = pcloud::Scene::CantOpenPODFile;	
	}
	_listvalid = -LIST_REFRESH_ITERATIONS;
	_depthlistvalid = -LIST_REFRESH_ITERATIONS;
	return scene;
}
//-----------------------------------------------------------------------------
pcloud::Scene *PointsScene::newScene(pcloud::IndexStream *stream, int &error, float uniform_filter_spacing)
{
	return pcloud::Scene::createFromPntsStream( stream, error, uniform_filter_spacing );
}
//-----------------------------------------------------------------------------
void PointsScene::getBounds(pt::BoundingBoxD &bb)
{
	//if (_boundsDirty)
	{
		_bb.clear();
		for (int i=0; i<_scenes.size() ; i++)
		{
//			_scenes[i]->update();
			_bb.expandBy(_scenes[i]->projectBounds().bounds());
		}
		_boundsDirty = false;
	}
	bb = _bb;
}
//-----------------------------------------------------------------------------
// Clear all point scenes from engine. freememory is from virtual func but not used
//-----------------------------------------------------------------------------
void PointsScene::clear(bool freeMemory)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::unique_lock<std::mutex> listlock(_listusemutex);

	/* notify observers */ 
	for (uint sc=0; sc<_scenes.size(); sc++)
	{
		FILE_OBS_LIST::iterator i = _fileObs.begin();
		while (i != _fileObs.end())
		{
			(*i)->sceneRemove( _scenes[sc] );
			++i;
		}

		delete _scenes[sc];
	}

	/* clear leaf node list */ 
	_voxels.clear();

	++_iteration;

	_scenes.clear();
	_listvalid = -LIST_REFRESH_ITERATIONS;
	_depthlistvalid = -LIST_REFRESH_ITERATIONS;
}
//-----------------------------------------------------------------------------
bool PointsScene::visitVoxels(PointsVisitor *visitor, bool load)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::unique_lock<std::mutex> llock(_listusemutex);

	pt::vector3d voxel2project(-pt::Project3D::project().registration().matrix()(3,0), 
		-pt::Project3D::project().registration().matrix()(3,1), 
		-pt::Project3D::project().registration().matrix()(3,2));

	for (uint s=0; s<_scenes.size(); s++)
	{
		pcloud::Scene* scene = _scenes[s];
		visitor->currentScene = scene;
		visitor->objCsBounds = scene->projectBounds().bounds();

		if (visitor->scene(_scenes[s]))
		{
			for (int c=0; c<scene->numObjects(); c++)
			{
				pcloud::PointCloud *cloud = scene->cloud(c);
				visitor->currentCloud = cloud;
				visitor->objCsBounds = cloud->projectBounds().bounds();
				
				if (visitor->cloud(cloud))
				{
					std::vector<pcloud::Voxel*> &voxels = cloud->voxels();
					for (uint v=0; v<voxels.size(); v++)
					{
						pcloud::Voxel *voxel = voxels[v];

						VoxelLoader loader(voxel, load ? 1.0f : -1.0f);

						visitor->objCsBounds = voxel->extents();
						visitor->objCsBounds.translateBy(voxel2project);
						visitor->currentVoxel = voxel;

						visitor->voxel(voxel);
					}
				}
			}
		}
	}
	return true;
}
//-----------------------------------------------------------------------------
void PointsScene::visitPointClouds( PointsVisitor *visitor )
{
	for (uint s=0; s<_scenes.size(); s++)
	{
		pcloud::Scene* scene = _scenes[s];
		visitor->currentScene = scene;
		visitor->objCsBounds = scene->projectBounds().bounds();

		if (visitor->scene(_scenes[s]))
		{
			for (int c=0; c<scene->numObjects(); c++)
			{
				pcloud::PointCloud *cloud = scene->cloud(c);
				visitor->currentCloud = cloud;
				visitor->objCsBounds = cloud->projectBounds().bounds();
				
				if (visitor->cloud(cloud))
					const_cast<pcloud::Node*>(cloud->root())->traverseTopDown(visitor);
			}
		}
	}
}
//-----------------------------------------------------------------------------
void PointsScene::visitNodes( PointsVisitor *visitor, bool visible_only ) 
{
	int numScenes = static_cast<int>(_scenes.size());
	for (int sc=0; sc<numScenes; sc++)
	{
		pcloud::Scene* scene = _scenes[sc];
		if (!visible_only || scene->displayInfo().visible())
		{
			visitor->objCsBounds = scene->projectBounds().bounds();
			if (!visitor->scene(scene)) continue;

			int numClouds = scene->size();
			for (int cl=0; cl<numClouds; cl++)
			{
				pcloud::PointCloud *cloud = scene->cloud(cl);
				visitor->objCsBounds = cloud->projectBounds().bounds();

				if (!visible_only || cloud->displayInfo().visible())
				{
					if (!visitor->cloud(cloud)) continue;
					const_cast<pcloud::Node*>(scene->cloud(cl)->root())->traverseTopDown(visitor);
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// File Observer
//-----------------------------------------------------------------------------
void PointsScene::addFileObserver( FileObserver *obs )
{
	_fileObs.push_back( obs );
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void PointsScene::remFileObserver( FileObserver *obs )
{
	FILE_OBS_LIST::iterator i = _fileObs.begin();

	while (i != _fileObs.end())
	{
		if (obs == *i)
		{
			_fileObs.erase(i);
			break;
		}
		++i;
	}
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void pointsengine::PointsScene::removeLOD(int clientID)
{
	VoxelLODRemoveVisitor	LODRemover(clientID);

	visitVoxels(&LODRemover);
}
