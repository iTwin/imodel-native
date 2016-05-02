#include "PointoolsVortexAPIInternal.h"
#include <pt/os.h>
#define POINTOOLS_API_BUILD_DLL
#include <gl/glew.h>

#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsVortexAPI_ResultCodes.h>
#include <ptapi/PointoolsAPI_handle.h>

#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/PointsExchanger.h>
#include <ptengine/VisibilityEngine.h>
#include <ptengine/engine.h>

#include <ptcloud2/pod.h>
#include <pt/project.h>

#include <ptgl/glcamera.h>
#include <ptgl/gltext.h>

#include <ptengine/queryScene.h>

#include <pt/trace.h>

using namespace pt;
using namespace pointsengine;

static pt::vector3d _lower;
static pt::vector3d _upper;

#define SCENE_MULTIPLIER	10000

pt::Project3D	*	_project = NULL;

extern int setLastErrorCode( int );


PTbool initializeScenesProject(void)
{
	if(_project == NULL)
	{
		_project = new Project3D;
	}

	return _project != NULL;
}

//-----------------------------------------------------------------------------
// handle management
//-----------------------------------------------------------------------------
PThandle PTAPI  ptGetCloudHandleByIndex(PThandle scene, PTuint cloud_index)
{
	return _GetCloudHandleByIndex(scene, cloud_index);
}
//-----------------------------------------------------------------------------
// ptGetNumCloudsInScene
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetNumCloudsInScene(PThandle scene)
{
	const pcloud::Scene *sc = thePointsScene().sceneBySceneOrCloudKey(scene);
	return sc ? sc->size() : 0;
}
//-----------------------------------------------------------------------------
// internal use
//-----------------------------------------------------------------------------
pcloud::Scene* sceneFromHandle(PThandle handle)
{
	return thePointsScene().sceneBySceneOrCloudKey(handle);
}
//-----------------------------------------------------------------------------
pcloud::PointCloud* cloudFromHandle(PThandle cloud)
{
	return thePointsScene().cloudByKey(cloud);
}
//-----------------------------------------------------------------------------
const pcloud::Scene* constSceneFromHandle(PThandle handle)
{
	return thePointsScene().sceneBySceneOrCloudKey(handle);
}
//-----------------------------------------------------------------------------
const pcloud::PointCloud* constCloudFromHandle(PThandle cloud)
{
	return thePointsScene().cloudByKey(cloud);
}
//-----------------------------------------------------------------------------
PTres		PTAPI	ptSetCloudTransform( PThandle cloud, const PTdouble *transform4x4, bool row_order )
{
	PTTRACE_FUNC

	pcloud::PointCloud *pcloud = cloudFromHandle(cloud);
	if (!pcloud)
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );	
	}

	mmatrix4d m;
	memcpy(m.data(), transform4x4, sizeof(PTdouble)*16);
	if (!row_order) m.transpose();

	pcloud->transform().useMatrix( m );
	pcloud->localBounds().dirtyBounds();
	pcloud->projectBounds().dirtyBounds();

	return setLastErrorCode( PTV_SUCCESS );	
}
//-----------------------------------------------------------------------------
PTres  PTAPI ptSetSceneTransform( PThandle scene, const PTdouble *transform4x4, bool row_order )
{
	PTTRACE_FUNC

	pcloud::Scene *sc = sceneFromHandle( scene );
	if (!scene)
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );   
	}

	unsigned int numClouds = ptGetNumCloudsInScene(scene);
	PThandle  cloudH;
	unsigned int c;

	mmatrix4d m;
	memcpy(m.data(), transform4x4, sizeof(PTdouble)*16);
	if (!row_order) m.transpose();

	for(c = 0; c < numClouds; c++)
	{
		pcloud::PointCloud *cloud = cloudFromHandle(ptGetCloudHandleByIndex(scene, c));

		if(cloud)
		{
			cloud->transform().useMatrix( m );
			cloud->localBounds().dirtyBounds();
			cloud->projectBounds().dirtyBounds();
			cloud->computeBounds();
		}
	}
	sc->localBounds().dirtyBounds();
	sc->projectBounds().dirtyBounds();
	sc->computeBounds();

	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION ); 
	return setLastErrorCode( PTV_SUCCESS ); 
}
//-----------------------------------------------------------------------------
PTres		PTAPI	ptGetCloudTransform( PThandle cloud, PTdouble *transform4x4, bool row_order )
{
	pcloud::PointCloud *pcloud = cloudFromHandle(cloud);
	if (!pcloud)
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );						
	}

	mmatrix4d mat = pcloud->transform().matrix();
	if (!row_order) mat.transpose();

	memcpy(transform4x4, mat.data(), sizeof(double)*16);

	return setLastErrorCode( PTV_SUCCESS );	
}
//-----------------------------------------------------------------------------
PTres		PTAPI	ptGetSceneTransform( PThandle scene, PTdouble *transform4x4, bool row_order )
{
	pcloud::Scene *sc = sceneFromHandle( scene );
	if (!scene)
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
	// return the sub-clouds transform
	// this is not correct, but it is the best we can do
	// until the transform system is updated from Bentely Pointools

	mmatrix4d mat = sc->cloud(0)->transform().matrix();
	if (!row_order) mat.transpose();

	memcpy(transform4x4, mat.data(), sizeof(double)*16);

	return setLastErrorCode( PTV_SUCCESS );	
}
//-------------------------------------------------------------------------------
// ptSceneHandles
//-------------------------------------------------------------------------------
PTint PTAPI ptGetSceneHandles(PThandle *handles)
{
	int numScenes = thePointsScene().size();

	for (int i=0; i<numScenes; i++)
	{
		handles[i] = thePointsScene()[i]->key();
	}
	return numScenes;
}
//-------------------------------------------------------------------------------
// ptGetLowerBound
//-------------------------------------------------------------------------------
PTbool PTAPI ptGetLowerBound(PTdouble *lower)
{
	BoundingBoxD bb;

	if (thePointsScene().size())
	{
		thePointsScene().getBounds(bb);
		lower[0] = bb.lower(0);
		lower[1] = bb.lower(1);
		lower[2] = bb.lower(2);
		return true;
	}
	return false;
}
//-------------------------------------------------------------------------------
// ptGetUpperBound
//-------------------------------------------------------------------------------
PTbool PTAPI ptGetUpperBound(PTdouble *upper)
{
	BoundingBoxD bb;

	if (thePointsScene().size())
	{
		thePointsScene().getBounds(bb);
		upper[0] = bb.upper(0);
		upper[1] = bb.upper(1);
		upper[2] = bb.upper(2);
		return true;
	}
	return false;
}
//-------------------------------------------------------------------------------
// ptGetUpperBound
//-------------------------------------------------------------------------------
PTint	PTAPI ptNumScenes()
{
	return thePointsScene().size();
}
//-------------------------------------------------------------------------------
// ptGetUpperBound
//-------------------------------------------------------------------------------
PTbool	PTAPI ptSceneInfo(PThandle scene, PTstr name, PTint &clouds, PTuint &num_points, PTuint &spec, PTbool &loaded, PTbool &visible)
{
	PTTRACE_FUNC

	const pcloud::Scene *sc = sceneFromHandle(scene);
	if (sc)
	{
#ifdef UNICODE
		wcscpy(name, sc->identifier());
#else
		wcscpy(name, pt::Unicode2Ascii::convert(sc->identifier()).c_str());
#endif
		clouds = sc->size();
		num_points = 0;
		
		spec = 0;

		for (int i=0; i<clouds;i++)
		{
			num_points += (PTuint)sc->cloud(i)->numPoints();
			spec |= sc->cloud(i)->hasChannel( pcloud::PCloud_Intensity ) ? PT_HAS_INTENSITY : 0;
			spec |= sc->cloud(i)->hasChannel( pcloud::PCloud_RGB ) ? PT_HAS_RGB : 0;
			spec |= sc->cloud(i)->hasChannel( pcloud::PCloud_Normal ) ? PT_HAS_NORMAL : 0;
			spec |= sc->cloud(i)->hasChannel( pcloud::PCloud_Classification ) ? PT_HAS_CLASSIFICATION : 0;
		}
		
		visible = sc->displayInfo().visible();
		loaded = sc->loaded();

		return setLastErrorCode( PTV_SUCCESS );	
	}
	return setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptUnloadScene
//-------------------------------------------------------------------------------
PTres PTAPI ptUnloadScene( PThandle scene )
{
	PTTRACE_FUNC_P1( scene )

	pcloud::Scene *sc = sceneFromHandle(scene);
	if (!sc) 
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
	sc->unload();
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// ptReloadScene
//-------------------------------------------------------------------------------
PTres PTAPI ptReloadScene( PThandle scene )
{
	PTTRACE_FUNC_P1( scene )

	pcloud::Scene *sc = sceneFromHandle(scene);
	if (!sc) 
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}	
	sc->reload();
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// ptIsSceneVisible
//-------------------------------------------------------------------------------
PTbool PTAPI ptIsSceneVisible( PThandle scene )
{
	const pcloud::Scene *sc = sceneFromHandle(scene);
	if (!sc) 
	{
		setLastErrorCode( PTV_INVALID_HANDLE );
	}
	return (sc ? sc->displayInfo().visible() : false);
}
//-------------------------------------------------------------------------------
// ptIsCloudVisible
//-------------------------------------------------------------------------------
PTbool PTAPI ptIsCloudVisible( PThandle scene )
{
	const pcloud::PointCloud* pc = cloudFromHandle(scene);
	if (!pc) 
	{
		setLastErrorCode( PTV_INVALID_HANDLE );
	}
	return (pc ? pc->displayInfo().visible() : false);
}
//-------------------------------------------------------------------------------
// ptGetUpperBound
//-------------------------------------------------------------------------------
const PTstr	PTAPI ptSceneFile( PThandle scene )
{
	const pcloud::Scene *sc = sceneFromHandle(scene);
	if (sc)
	{
		return sc->filepath().path();
	}
	setLastErrorCode( PTV_INVALID_HANDLE );
	return 0;
}
//-------------------------------------------------------------------------------
// ptGetUpperBound
//-------------------------------------------------------------------------------
PTres	PTAPI ptCloudInfo(PThandle cloud, PTstr name, PTuint &num_points, PTuint &spec, PTbool &visible)
{
	const pcloud::PointCloud *pc = cloudFromHandle(cloud);
	if (pc)
	{
		wcscpy(name, /*pt::Unicode2Ascii::convert(*/pc->identifier()/*).c_str()*/);
		visible = pc->displayInfo().visible();
		num_points = (PTuint)pc->numPoints();

		spec = 0;

		spec |= pc->hasChannel( pcloud::PCloud_Intensity ) ? PT_HAS_INTENSITY : 0;
		spec |= pc->hasChannel( pcloud::PCloud_RGB ) ? PT_HAS_RGB : 0;
		spec |= pc->hasChannel( pcloud::PCloud_Normal ) ? PT_HAS_NORMAL : 0;
		spec |= pc->hasChannel( pcloud::PCloud_Classification ) ? PT_HAS_CLASSIFICATION : 0;

		return setLastErrorCode( PTV_SUCCESS );	
	}
	return setLastErrorCode( PTV_INVALID_HANDLE );	
}
//-------------------------------------------------------------------------------
// ptGetProxyPoints
//-------------------------------------------------------------------------------
PTuint	PTAPI PTAPI ptGetCloudProxyPoints(PThandle cloud, PTint num_points, PTfloat *pnts, PTubyte *col)
{
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
	return 0;
}
//-------------------------------------------------------------------------------
using namespace pcloud;
struct PointLimit
{
	PointLimit(float _lod) : pcount(0), lod(_lod) {}
	
	template<class T> bool operator()(const T &pnt)	{
		++pcount;
		return pcount < limit;
	}
	void setVoxel(Voxel *vox) 
	{ 
		limit = vox->fullPointCount() * lod;
		pcount = 0;
	}
	int pcount; 
	int limit;	
	float lod;
};
//-------------------------------------------------------------------------------
struct ReadPoints
{
	ReadPoints(PTfloat *_pnts, PTubyte *_col, PTint _limit) 
		: pnts(_pnts), col(_col), pcount(0), limit(_limit)	{}

	PTfloat *pnts;
	PTubyte *col;
	PTint pcount;
	PTint limit;
	PTdouble *cb;

	void point(const vector3d &pnt, int index)
	{
		if (pcount < limit)
		{
			pnts[pcount*3] = pnt.x + cb[0];
			pnts[pcount*3+1] = pnt.y + cb[1];
			pnts[pcount*3+2] = pnt.z + cb[2];
		}
		++pcount;
	}
	void point(const vector3 &pnt, int index)
	{
		if (pcount < limit)
		{
			pnts[pcount*3] = pnt.x + cb[0];
			pnts[pcount*3+1] = pnt.y + cb[1];
			pnts[pcount*3+2] = pnt.z + cb[2];
		}
		++pcount;
	}
	inline bool setVoxel(Voxel *vox) { return true; }
};
//-------------------------------------------------------------------------------
// ptGetProxyPoints
//-------------------------------------------------------------------------------
PTuint	PTAPI PTAPI ptGetSceneProxyPoints(PThandle scene, PTint num_points, PTfloat *pnts, PTubyte *col)
{
	PTTRACE_FUNC

	debugAssertM(!col, L"Support for Colour in proxy points is not implemented");

	const Scene *sc = scene ? sceneFromHandle(scene) : 0;

	if (sc || !scene)
	{
		int state;

		// return the voxel extent corners
		PointsScene::VOXELSLIST voxels;
		PointsScene::UseSceneVoxels voxelslock(voxels, state);
		PointsScene::VoxIterator it = voxels.begin(); 

		pt::vector3d world2prj;
		world2prj.set(-pt::Project3D::project().registration().matrix()(3,0),
				-pt::Project3D::project().registration().matrix()(3,1),
				-pt::Project3D::project().registration().matrix()(3,2));

		int num_actual_pnts=0;
	
		// count voxels in scene
		while (it != voxels.end())
		{
			if (!sc || (*it)->pointCloud()->scene() == sc)
				num_actual_pnts += 8;

			++it;
		};
		it = voxels.begin(); 
		int count = 0;

		while (it != voxels.end())
		{
			if (!sc || (*it)->pointCloud()->scene() == sc)
			{
				const pcloud::Voxel *v = (*it);
				BoundingBoxD bb = v->extents();
				bb.translateBy( world2prj );
				
				vector3d pnt;

				// extract corners
				for (int i=0; i<8; i++)
				{
					bb.getExtrema(i, pnt);
				
					pnts[count++] =  pnt.x;
					pnts[count++] =  pnt.y;
					pnts[count++] =  pnt.z;
				}

				if (count / 3 == num_points) 
					break;
			}
			++it;
		}
		num_actual_pnts = count / 3;

		/* 1 / 255 is the smallest lod possible */ 
/*
		int state;
		PointsScene::VOXELSLIST voxels;
		PointsScene::UseSceneVoxels voxelslock(voxels, state);
		PointsScene::VoxIterator it = voxels.begin(); 

		//
		PointLimit plimit(lod);
		ReadPoints readPoints(pnts, col, num_points);

		PTdouble cb[3];
		ptGetCoordinateBase( cb );
		readPoints.cb = cb;

		thePointsPager().pause();

		while (it != voxels.end())
		{
			Voxel *vox = *it;
			
			if (vox->pointCloud()->scene() == sc)
			{
				VoxelLoader loader( vox, lod < 0.0039216 ? 0.0039216 : lod );
				plimit.setVoxel(vox);
				pointsengine::IterateVoxelPoints::
					iteratePoints(vox, plimit, readPoints, pt::ProjectSpace);
			}
			++it;
		}
*/
		return num_actual_pnts;
	}
	setLastErrorCode( PTV_INVALID_HANDLE );
	return 0;	
}

//-------------------------------------------------------------------------------
// ptSceneBounds
//-------------------------------------------------------------------------------
PTres	PTAPI ptSceneBounds(PThandle scene, PTfloat *lower, PTfloat *upper)
{
	const pcloud::Scene *sc = sceneFromHandle(scene);
	
	/* TODO: use all scenes */ 
	if (!sc && thePointsScene().size()) 
		sc = thePointsScene()[0];

	if (sc)
	{
		BoundingBoxD bb = sc->projectBounds().bounds();
		lower[0] = bb.lower(0);
		lower[1] = bb.lower(1);
		lower[2] = bb.lower(2);
		upper[0] = bb.upper(0);
		upper[1] = bb.upper(1);
		upper[2] = bb.upper(2);
		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptSceneBoundsd
//-------------------------------------------------------------------------------
PTres	PTAPI ptSceneBoundsd(PThandle scene, PTdouble *lower, PTdouble *upper)
{
	const pcloud::Scene *sc = sceneFromHandle(scene);

	/* TODO: use all scenes */ 
	if (!sc && thePointsScene().size()) 
		sc = thePointsScene()[0];

	if (sc)
	{
		BoundingBoxD bb = sc->projectBounds().bounds();
		lower[0] = bb.lower(0);
		lower[1] = bb.lower(1);
		lower[2] = bb.lower(2);
		upper[0] = bb.upper(0);
		upper[1] = bb.upper(1);
		upper[2] = bb.upper(2);
		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptCloudBounds
//-------------------------------------------------------------------------------
PTres	PTAPI ptCloudBounds(PThandle cloud, PTfloat *lower, PTfloat *upper)
{
	const pcloud::PointCloud *pc = cloudFromHandle(cloud);

	if (pc)
	{
		BoundingBoxD bb = pc->projectBounds().bounds();
		lower[0] = bb.lower(0);
		lower[1] = bb.lower(1);
		lower[2] = bb.lower(2);
		upper[0] = bb.upper(0);
		upper[1] = bb.upper(1);
		upper[2] = bb.upper(2);
		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptCloudBoundsd
//-------------------------------------------------------------------------------
PTres	PTAPI ptCloudBoundsd(PThandle cloud, PTdouble *lower, PTdouble *upper)
{
	const pcloud::PointCloud *pc = cloudFromHandle(cloud);

	if (pc)
	{
		BoundingBoxD bb = pc->projectBounds().bounds();
		lower[0] = bb.lower(0);
		lower[1] = bb.lower(1);
		lower[2] = bb.lower(2);
		upper[0] = bb.upper(0);
		upper[1] = bb.upper(1);
		upper[2] = bb.upper(2);
		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptShowScene
//-------------------------------------------------------------------------------
PTres	PTAPI ptShowScene(PThandle scene, PTbool visible)
{
	PTTRACE_FUNC_P2( scene, visible )

	pcloud::Scene *sc = sceneFromHandle(scene);
	if (sc)
	{
		sc->displayInfo().visible(visible);
		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptShowCloud
//-------------------------------------------------------------------------------
PTres	PTAPI ptShowCloud(PThandle cloud, PTbool visible)
{
	PTTRACE_FUNC_P2( cloud, visible )

	pcloud::PointCloud *pc = cloudFromHandle(cloud);
	if (pc) 
	{
		pc->displayInfo().visible(visible);
		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptRemoveScene
//-------------------------------------------------------------------------------
PTres	PTAPI ptRemoveScene(PThandle scene)
{
	PTTRACE_FUNC_P1( scene )

	pcloud::Scene*sc = sceneFromHandle(scene);
	if (sc)
	{
		PTdouble coordinateBase[3];

		extern PTenum g_autoBaseMethod;

		if (g_autoBaseMethod & PT_AUTO_BASE_FIRST_ONLY &&
			pt::Project3D::project().numObjects() > 1)
		{
			ptGetCoordinateBase( coordinateBase );
		}

		pointsengine::thePointsPager().pause();
		pt::Project3D::project().removeScene(sc);
		thePointsScene().removeScene(sc, true);

		pointsengine::thePointsPager().unpause();

		if (g_autoBaseMethod & PT_AUTO_BASE_FIRST_ONLY)
		{
			ptSetCoordinateBase( coordinateBase );
		}

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-------------------------------------------------------------------------------
// ptGetUpperBound
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptNormaliseCoordinateSystem(void)
{
	PTTRACE_FUNC

	_project->optimizeCoordinateSpace();
}
PTvoid PTAPI ptProjectToWorldCoords( PTdouble *v )
{
	pt::vector3d vp(v), vw;
	_project->project2WorldSpace( vp, vw );
	vw.get(v);
}
PTvoid PTAPI ptWorldToProjectCoords( PTdouble *v )
{
	pt::vector3d vp, vw(v);
	_project->world2ProjectSpace( vw, vp );
	vp.get(v);
}