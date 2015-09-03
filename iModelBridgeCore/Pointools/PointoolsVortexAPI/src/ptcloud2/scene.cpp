#include <windows.h>
#include <ptcloud2\scene.h>
#include <iostream>

#include <utility\ptstr.h>

using namespace pcloud;

namespace pcloud
{
	typedef std::map<PointCloudGUID, PointCloud*> CloudsByGUID;
};
/***
Create a POD Scene from a Points Stream object (IndexStream). This may take time to complete
and significant IO.

Static creation method enables some error handling
*/
Scene *Scene::createFromPntsStream( IndexStream *pntsStream, int &error, float uniform_filter_spacing )
{
	//if (pntsStream->getNumCloudPoints() < min_points) return 0;

	Scene *sc = new Scene( pntsStream, error, uniform_filter_spacing );
	if (error != Success)
	{
		delete sc;
		return 0;
	}
	return sc;
}
/***
Create a POD Scene from a POD File, essentially opening a POD file.

Static creation method enables some error handling (TODO!)
*/
Scene *Scene::createFromFile( const ptds::FilePath &path )
{
	Scene *sc = new Scene( path );
	return sc;
}
/***
POD Scene constructor
*/
Scene::Scene(const ptds::FilePath &path)
{
	/*take absolute path*/ 
	wchar_t b[PT_MAXPATH];
	path.fullpath(b);
	m_filepath.setPath(b);
	m_filepath.setAbsolute();
	_instance = 0;
	_loaded = true;
	_editStateID = 0;
}
/***
POD Scene destructor
*/
Scene::~Scene() 
{
	clear();
}

// object guid
// based on the guid of the first point cloud 
pt::Guid Scene::objectGuid() const
{
	if (_clouds.size())
	{
		pt::Guid g(_clouds[0]->guid());
		g.setPart2(1);

		return g;
	}
	return pt::Guid();	// null
}

void Scene::setInstanceIndex( int instance )
{
	_instance = instance;
}

PointCloud* Scene::newCloud( __int64 guid )
{ 
	PointCloud *pc = new PointCloud(L"Cloud", this);
	
	pc->setGuid( guid );
	if (!guid) pc->generateGuid();

	return pc;
}
ScanPosition* Scene::addScanPosition(const mmatrix4d &mat)
{
	if (mat == mmatrix4d::identity()) return 0;

	wchar_t name[64];
	/* check if this position already exists */ 
	for (int i=0; i<_scanpositions.size(); i++)
	{
		if (mat == _scanpositions[i]->registration().matrix())
			return _scanpositions[i];
	}
	swprintf(name, L"Scan Position %d", _scanpositions.size());
	ScanPosition *sp = new ScanPosition(mat, name);

	_scanpositions.push_back(sp);
	return sp;
}
void Scene::addCloud(PointCloud* cloud)
{		
	if (!cloud->guid()) cloud->generateGuid();

	_clouds.push_back(cloud);
	_cloudsByGUID.insert( CloudsByGUID::value_type(cloud->guid(), cloud) );

	cloud->parent(this);
	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();
}
void Scene::deleteCloud(PointCloud* cloud)
{
	removeCloud(cloud);
	delete cloud;
}
void Scene::removeCloud(PointCloud* cloud)
{
	debugAssertM(0, "Scene::removeCloud(PointCloud* cloud) has not been implemented");
	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();	
}
void Scene::reload()
{
	_loaded = true;
}
void Scene::unload()
{
	_loaded = false;
}
void Scene::clear() 
{
	for (std::vector<PointCloud*>::iterator a=_clouds.begin();
		a!=_clouds.end(); a++) delete (*a);

	for (std::vector<ScanPosition*>::iterator b=_scanpositions.begin();
		b!=_scanpositions.end(); b++) delete (*b);

	_clouds.clear();
	_cloudsByGUID.clear();

	_scanpositions.clear();
	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();
}
/*****************************************************************************/
/**
* @brief
* @return __int64
*/
/*****************************************************************************/
__int64 Scene::fullPointCount() const
{
	__int64 pcount = 0;
	for (int i= 0; i<_clouds.size(); i++)
	{
		pcount += _clouds[i]->numPoints();
	}
	return pcount;
}
/*****************************************************************************/
/**
* @brief
* @return __int64
*/
/*****************************************************************************/
__int64 Scene::lodPointCount() const
{
	__int64 pcount = 0;
	for (int i= 0; i<_clouds.size(); i++)
	{
		if (_clouds[i]->root())	
			pcount += _clouds[i]->root()->calcLodPointCount();
	}
	return pcount;
}
const char* Scene::creationErrorText( int errorCode )
{
	switch (errorCode)
	{
		case Scene::ReadPntStreamFailure:
			return "Error whilst reading points stream (Error -100)";

		case Scene::OutOfMemory:
			return "Out of Memory (Error -101)";

		case Scene::FileWriteError:
			return "File write error (Error -102)";

		case Scene::NoPointsInStream:
			return "No points in import stream (Error -103)";

		case Scene::ReadStreamError:
			return "Failed to read points stream (Error -104)";

		case Scene::CantOpenPODFile:
			return "Failed to open POD file (Error -105)";

		case Scene::Success:
			return "Success";

		case Scene::PODVersionNotHandled:
			return "POD version not handled (Error -107)";

		case Scene::InvalidPODFile:
			return "POD invalid or corrupt (Error -106)";

		default:
			return "Unknown Error";
	}
	return 0;
}
int		Scene::editStateID() const				
{ 
	return  _editStateID; 
}
void	Scene::setEditStateID( int id )			
{ 
	_editStateID=id; 
}