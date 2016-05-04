/*----------------------------------------------------------*/ 
/* PointsScene.h											*/ 
/* Point Scene Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_POINTCLOUD_SCENE
#define POINTOOLS_POINTCLOUD_SCENE 1


#include <ptcloud2/scene.h>
#include <ptengine/ptengine_api.h>
#include <ptengine/pointsvisitor.h>

#include <list>
#include <vector>

namespace pcloud { class Voxel; };

namespace pointsengine
{
struct FileObserver
{
	virtual void sceneRemove( pcloud::Scene *Scene )=0;
	virtual void sceneAdd( pcloud::Scene *Scene )=0;
};

class PTENGINE_API PointsScene
{
public:
	PointsScene();
	~PointsScene();

	pcloud::Scene *openScene(const ptds::FilePath &path, int &error);
	pcloud::Scene *newScene(pcloud::IndexStream *stream, int &error, float uniform_filter_spacing=-1.0f);

	void addScene(pcloud::Scene*);
	void removeScene(pcloud::Scene*, bool freeMemory);
	
	pcloud::Scene *operator [] (int i);
	int size();

	void clear(bool freeMemory=true);

	/*Visible Voxel List	*/ 
	/*read only list*/ 
	typedef std::list<pcloud::Voxel*> VOXELSLIST;
	typedef VOXELSLIST::iterator VoxIterator;

	class PTENGINE_API UseSceneVoxels 
	{ 
	public: 
		UseSceneVoxels(VOXELSLIST &vlist, int &state); 
		~UseSceneVoxels(); 
		
		VOXELSLIST	&_vlist;
		bool hasLock;
	};
	class PTENGINE_API UseDepthSortedVoxels
	{
	public:
		UseDepthSortedVoxels(VOXELSLIST &vlist, int &state);
		~UseDepthSortedVoxels();

		VOXELSLIST	&_vlist;
		bool hasLock;
	};

	bool visitVoxels(PointsVisitor *visitor, bool load = false);
	bool visitPoints(PointsVisitor *visitor, pt::CoordinateSpace &cs, bool load = false);
	void visitNodes(PointsVisitor *visitor, bool visible_only = true );
	void visitPointClouds( PointsVisitor *visitor );

	VoxIterator voxbegin();
	VoxIterator voxend();

	const VOXELSLIST &voxels();

	void depthSortVoxels();

	void getBounds(pt::BoundingBoxD &bb);

	void removeLOD(int clientID);
	
	bool initialize();

	void setFlagGlobal(const pcloud::Flag &flag);

	const pcloud::Scene * sceneBySceneOrCloudKey(const pt::ObjectKey &objk) const;
	pcloud::Scene * sceneBySceneOrCloudKey(const pt::ObjectKey &objk);

	const pcloud::PointCloud *cloudByKey(const pt::ObjectKey &objk) const;
	pcloud::PointCloud *cloudByKey(const pt::ObjectKey &objk);

	const pcloud::PointCloud *cloudByGUID(const int64_t guid) const;
	pcloud::PointCloud *cloudByGUID(const int64_t guid);

	int getIteration() const { return _iteration; }

	void addFileObserver( FileObserver *obs );
	void remFileObserver( FileObserver *obs );

	bool PointsScene::getAllCloudRoots(std::vector<pcloud::Node *> &result, bool visibleOnly);


private:
	bool _boundsDirty;
	pt::BoundingBoxD _bb;

	/* session key */ 
	std::map<pt::ObjectKey, pcloud::Scene*> _sceneByKey;

	/* guid key */ 
	std::map<int64_t, pcloud::PointCloud*> _cloudByGuid;

	/* remove file notification */ 
	std::list< FileObserver * > _fileObs;

    std::mutex _mutex;
	int	_iteration;
};
}
#endif