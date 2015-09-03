#pragma once

#include <pt/typedefs.h>
#include <pt/datatree.h>
#include <ptcloud2/scene.h>
#include <ptcloud2/pointcloud.h>
#include <ptedit/constriants.h>

namespace ptedit
{

enum SelectionMode
{
	SelectPoint = 1,
	DeselectPoint = 2,
	UnhidePoint = 3
};

union LayerState
{
	LayerState(ubyte current, ubyte active, ubyte locked, ubyte visible)
	{ 
		layers[0] = current; 
		layers[1] = active; 
		layers[2] = locked; 
		layers[3] = visible; 
	}
	LayerState(__int64 state) : layersInt(state) {}

	ushort layers[4];
	__int64 layersInt;
};

struct GlobalState
{
	GlobalState();

	void readState( const pt::datatree::Branch* b );
	void writeState( pt::datatree::Branch* b );

	/* stored state */ 
	bool						autoDeselect;
	SelectionMode				selmode;

	inline bool						inScope( const pcloud::PointCloud *cloud ) const
	{
		if (!cloud || !cloud->displayInfo().visible()) 
			return false;		

		if (execSceneScope && cloud->scene() != execSceneScope) 
			return false;
		
		if (!scope) 
			return true;

		return	scopeIsScene ? inScope( cloud->scene() ) : 
				((scope && 
				cloud->guid() == scope &&
				cloud->scene()->getInstanceIndex() == scopeInstance 
				) ? true : false);
	}
	// in scope check
	inline bool						inScope( const pcloud::Scene *scene ) const
	{
		if (!scene || !scene->displayInfo().visible()) 
			return false;		

		if (execSceneScope && execSceneScope != scene) 
			return false;

		if (!scope) 
			return true;

		if (!scopeIsScene) 
			return true;	/* will get dismissed later on cloud check */ 

		return	(scope && scopeIsScene && 
			scene->cloud(0)->guid() == scope &&
			scene->getInstanceIndex() == scopeInstance 
			) ? true : false;
	}
	// exclusion is stronger than scope, ie. operations that do not respect scope still must respect exclusion
	inline bool						isSceneExcluded( const pcloud::Scene *scene ) const
	{
		return (execSceneScope && execSceneScope != scene) ? true : false;
	}

	void setExecutionScope( pcloud::Scene *scene )
	{
		execSceneScope = scene;
	}

	pcloud::PointCloudGUID		scope;				// scope that limits following edit operations 
	bool						scopeIsScene;	
	int							scopeInstance;

	/* application state */ 
	bool						multithreaded;
	float						density;

	EditConstraint::PointTestFunc	selPntTest;
	EditConstraint					*constraint;

private:
	LayerState					layer;
	pcloud::Scene*				execSceneScope;	// this scope is used to limit the entire execution - IT IS NOT WRITTEN IN THE STATE NODE
};
struct PreserveState
{
	PreserveState();
	~PreserveState();

	pt::datatree::Branch save;
};

extern GlobalState g_state;
extern ubyte g_activeLayers;
extern ubyte g_visibleLayers;
extern ubyte g_currentLayer;
extern ubyte g_lockedLayers;
}
