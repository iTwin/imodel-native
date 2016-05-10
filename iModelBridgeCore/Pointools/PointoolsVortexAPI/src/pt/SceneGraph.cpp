/*--------------------------------------------------------------------------*/ 
/*  Geometry.cpp															*/ 
/*	Geometry base class implementation										*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include "PointoolsVortexAPIInternal.h"

//#define FILE_TRACE 1

#include <pt/variant.h>
#include <pt/scenegraph.h>
#include <pt/sceneclassmanager.h>
#include <pt/project.h>
#include <pt/format.h>
#include <pt/ptmath.h>
#include <pt/trace.h>


#include <map>
#include <iostream>

using namespace pt;
ptds::FileType NULL_FILETYPE("Null", "Null");

#define EMPTY_BOUNDING_BOX BoundingBox(10,-10,10,-10,10,-10)

class TraceOutput : public Output
{
public:
	TraceOutput() { _level = 0; };
	virtual ~TraceOutput() { _level = 0; };
	void CDECL_ATTRIBUTE outputf(const char* fmt ...) const;
	void CDECL_ATTRIBUTE outputLinef(const char* fmt ...) const;

protected:
	int _level;
private:
	void tab() const;
};
//#define DEBUGGING_SG

//-----------------------------------------------------------------------------
// session key
namespace pt
{
	std::set<ObjectKey>	g_sessionKeys;			// for when sessions are restored to ensure keys are not duplicated
	ObjectKey			g_globalSessionObjInstanceKey = 0;
}
//-----------------------------------------------------------------------------
Object3D::Object3D(const wchar_t *id, const Object3D* owner)  
	: Object(id),  m_parent(0),
	m_localBounds(std::bind(&Object3D::computeBounds, this)),
	m_projectBounds(std::bind(&Object3D::computeBounds, this))
{
	parent(owner); 

	m_key = generateKey(this);
}
//-----------------------------------------------------------------------------
int	Object3D::depth() const
{
	int d=0;

	const Object3D *p = Object3D::parent();

	while (p)
	{
		++d;
		p = p->parent();
	}
	return d;
}
//-----------------------------------------------------------------------------
ObjectKey Object3D::generateKey( const Object3D *obj )
{
	int d = obj->depth();
	int m = 1;
	if (d==1) m = 1000;	
	else if (d>1) m = 1000000;

	ObjectKey newKey = g_globalSessionObjInstanceKey;
	std::set<ObjectKey>::iterator i;

	do
	{
		i = g_sessionKeys.find(++newKey*m);
	}
	while (i!=g_sessionKeys.end());

	g_sessionKeys.insert(newKey*m);
	g_globalSessionObjInstanceKey = newKey+1;
	return newKey*m;
}
//-----------------------------------------------------------------------------
// parent()
/// set the objects parent
/*!
 * Correctly parents/unparents the child object setting up the transformation
 * to be invariant. An unparented object's transform will always transform to
 * world space. 
 *
 * Also manages bounds computation
 */
//-----------------------------------------------------------------------------
void Object3D::parent(const Object3D *par)
{
	PTTRACE("Object3D::parent");

	if (!par && !m_parent) return;
	
	/* unparent this object, its transform must transform to world space */ 
	mmatrix4d Mw;
	m_registration.compileMatrix(Mw, WorldSpace);

	if (m_parent)
	{
		const_cast<Bounds3D*>(&m_parent->localBounds())->dirtyBounds();
		const_cast<Bounds3DD*>(&m_parent->projectBounds())->dirtyBounds();
	}
	if (!par)
	{
		m_registration.matrix(Mw);
		m_registration.parent(0);
		m_parent = 0;
	}
	else
	{
		m_registration.parent(const_cast<Transform*>(&par->registration()));
		mmatrix4d Ms = par->registration().matrix();
		Ms.invert();
		Mw >>= Ms;
		m_registration.matrix(Mw);
		m_parent = par;
	}
	m_registration.compileMatrix(ProjectSpace, true);
}
//-----------------------------------------------------------------------------
// Bounds Computation | generic implementation
//-----------------------------------------------------------------------------
void Group3D::_computeBounds()
{
	PTTRACE("Group3D::computeBounds");

	m_localBounds.clearBounds();
	m_projectBounds.clearBounds();

	/*traverse objects*/ 
	int i=0;
	Bounds3D b(nullptr);
	while (i<numObjects())
	{
		const Object3D* o = object(i++);

		if (o->localBounds().isValid() && !o->localBounds().bounds().isEmpty())
		{
			o->localBounds().transformedBounds(o->registration().matrix(), b);
			m_localBounds.expandBounds(b.bounds());
			
			m_registration.compileMatrix(pt::ProjectSpace);
		
			/*if (o->parent())
			{
				o->projectBounds().transformedBounds(o->transform().cmpMatrix(), b);
				m_projectBounds.expandBounds(b.bounds());
			}
			else */m_projectBounds.expandBounds(o->projectBounds().bounds());
		}
	};
	m_localBounds.undirtyBounds();
	m_projectBounds.undirtyBounds();
}
//-----------------------------------------------------------------------------
// Bounds Computation | generic implementation
//-----------------------------------------------------------------------------
void Group3D::visibleBounds(Bounds3DD &bounds) const
{
	PTTRACE("Group3D::visibleBounds");

	/*traverse objects*/ 
	int i=0;
	while (i<numObjects())
	{
		const Object3D* o = object(i++);

		if (o->localBounds().isValid() && !o->localBounds().bounds().isEmpty() && o->displayInfo().visible())
		{
			bounds.expandBounds(o->projectBounds().bounds());
		}
	};
}
//-----------------------------------------------------------------------------
/// searches child objects for nearest point returning deistance to point
//-----------------------------------------------------------------------------
double Group3D::findNearestPoint(const pt::vector3d &pnt, pt::vector3d &nearest, CoordinateSpace cs) const
{
	PTTRACE("Group3D::findNearestPoint");

	/*traverse objects*/ 
	int i=0;

	double d = 0;
	double dist = -1;
	vector3d nr;

	while (i<numObjects())
	{
		const Object3D* o = object(i++);
			
#ifdef DEBUGGING_SG
		std::cout << "searching obj " << i << std::endl;
#endif
		if (cs==ProjectSpace)
		{
			if (o->projectBounds().bounds().inBounds(pnt))
			{
				d = o->findNearestPoint(pnt, nr, cs);
				if (dist < 0 || d<dist && d > 0)
				{
					dist = d;
					nearest = nr;
#ifdef DEBUGGING_SG
		std::cout << "* nearest " << std::endl;
#endif
				}
			}
		}
	};
	return dist;
}

#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
// drawGL | generic implementation
//-----------------------------------------------------------------------------
void Group3D::drawGL(uint32 drawmode, int millisecs, const ptgl::Viewport *viewport)
{
#ifndef POINTOOLS_POD_API
	PTTRACE("Group3D::drawGL");

	if (displayInfo().visible())
	{
		int i=0;
		while (i<numObjects())
		{
			Object3D* o = const_cast<Object3D*>(object(i++));
			if (o->displayInfo().visible() && !o->displayInfo().culled())
			{
				glMatrixMode(GL_MODELVIEW);
				o->registration().pushGL();
				o->drawGL(drawmode, millisecs, viewport);
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
			}
		};
	}
#endif
}
#endif
//-----------------------------------------------------------------------------
// resetCoordinateSpace()
/* resets all coorindate space manipulations (balancing)
   so that projectspace = worldspace
 */
//-----------------------------------------------------------------------------
void Group3D::resetCoordinateSpace()
{
	PTTRACE("Group3D::resetCoordinateSpace");

	for (int i=0; i<numObjects(); i++)
	{	
		Object3D *obj = const_cast<Object3D*>(object(i));
		if (obj->isGroup())
			static_cast<Group3D*>(obj)->resetCoordinateSpace();
		else 
		{
			mmatrix4d Mow;
			bool m = obj->registration().compileMatrix(Mow, WorldSpace);
			debugAssertM(m, "No path to world space");
			obj->registration().matrix(Mow);

			BoundingBoxD bbd(obj->localBounds().bounds().ux(), obj->localBounds().bounds().lx(), 
							obj->localBounds().bounds().uy(), obj->localBounds().bounds().ly(),
							obj->localBounds().bounds().uz(), obj->localBounds().bounds().lz());
			obj->projectBounds().clearBounds();
			obj->projectBounds().expandBounds(bbd);
			obj->projectBounds().transformBounds( Mow );
			obj->projectBounds().undirtyBounds();
			obj->localBounds().undirtyBounds();
		}
	}
	m_registration.identity();
	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();
}
//
// projectBoundsTransform
//
// transform the project bounds by the matrix
//
void Group3D::projectBoundsTransform(const mmatrix4d &m)
{
	PTTRACE("Group3D::projectBoundsTransform");

	for (int i=0; i<numObjects(); i++)
	{	
		Object3D *obj = const_cast<Object3D*>(object(i));
		obj->projectBounds().transformBounds(m);
		obj->projectBounds().undirtyBounds();

		if (obj->isGroup())
			static_cast<Group3D*>(obj)->projectBoundsTransform(m);

		obj->registration().compileMatrix(ProjectSpace, true);
	}	
}
/*
*/

double pt_mag(const double &v)
{
	double v1 = fabs(v);
	double m = v1;

	if (v1 < 0.000001) m = 0.000001;
	else if (v1 < 0.00001) m = 0.00001;
	else if (v1 < 0.0001) m = 0.0001;
	else if (v1 < 0.001) m = 0.001;
	else if (v1 < 0.01) m = 0.01;
	else if (v1 < 0.1) m = 0.1;
	else if (v1 < 1) m = 1;
	else if (v1 < 10) m = 10;
	else if (v1 < 100) m = 100;
	else if (v1 < 1000) m = 1000;
	else if (v1 < 10000) m = 10000;
	else if (v1 < 100000) m = 100000;
	else if (v1 < 1000000) m = 1000000;
	else if (v1 < 10000000) m = 10000000;
	else if (v1 < 100000000) m = 100000000;
	else if (v1 < 1000000000) m = 1000000000;
	return m;
}
//-----------------------------------------------------------------------------
// moves translations in the child object to the parent
// useful for minimising registrations of child object
//-----------------------------------------------------------------------------
bool Group3D::moveChildTranslationsToParent( double min_translation, double roundto )
{ 
	if (!numObjects()) return false;

	PTTRACE("Group3D::moveChildTranslationsToParent");

	vector3d av_trans(0,0,0);
	int i;

	/* perform this recursively, it need to start at the bottom */ 
	for (i=0; i<numObjects(); i++)
	{	
		Object3D *obj = const_cast<Object3D*>( object(i) );

		if (obj->isGroup())
			static_cast<Group3D*>(obj)->moveChildTranslationsToParent( min_translation, roundto );
	}
	/* get the average translation */ 
	for (i=0; i<numObjects(); i++)
	{
		Object3D *obj = const_cast<Object3D*>( object(i) );
		vector3d trans = obj->registration().translation();	

		av_trans += trans;
	}
	av_trans /= numObjects();

	/* check minimums */ 
	if (av_trans.x < min_translation) av_trans.x = 0;
	if (av_trans.y < min_translation) av_trans.y = 0;
	if (av_trans.z < min_translation) av_trans.z = 0;

	/* round translation to - used to get 'clean' translation */ 
	if (roundto > 0)
	{
		av_trans.x = iRound(av_trans.x / roundto) * roundto;
		av_trans.y = iRound(av_trans.y / roundto) * roundto;
		av_trans.z = iRound(av_trans.z / roundto) * roundto;
	}

	/* add the average translation to this */ 
	registration().translate( av_trans );
	projectBounds().translate( av_trans );

	/* remove this translation from child objects */ 
	for (i=0; i<numObjects(); i++)
	{
		Object3D *obj = const_cast<Object3D*>(object(i));
		obj->registration().translate( -av_trans );
		obj->projectBounds().dirtyBounds();
	}	
	return true;
}
//-----------------------------------------------------------------------------
// set the project space origin
//-----------------------------------------------------------------------------
void Project3D::optimizeCoordinateSpace()
{
	PTTRACE("Group3D::optimizeCoordinateSpace");

	BoundingBox bb( m_localBounds.bounds() );
	vector3 cenf( bb.center() );
	vector3d cend;
	project2WorldSpace( cenf, cend );

	double mag = pt_mag( bb.diagonal().length()/*cend.length()*/ );
	double mag_dist = pt_mag( cend.length() );
	mag *= 10;

	/* there's no point on data already near origin*/ 
	if (mag_dist < 1000) return;
	
	cend.x = iRound(cend.x / mag) * mag;
	cend.y = iRound(cend.y / mag) * mag;
	cend.z = iRound(cend.z / mag) * mag;

	setProjectSpaceOrigin( cend );
}
//-----------------------------------------------------------------------------
// set the project space origin
//-----------------------------------------------------------------------------
bool Project3D::setProjectSpaceOrigin( const pt::vector3d &origin )
{
	PTTRACE("Group3D::setProjectSpaceOrigin");

	assert(!m_parent);
	if (m_parent) return false;

	/* reset coordinate space so all objects are in world space */ 
	resetCoordinateSpace();

	if (origin.is_zero()) return true;

	/* move the child translations into the parent recursively */ 
	moveChildTranslationsToParent(0,0);

	/* check the result and adjust the final translation from project -> world			*/ 
	/* be the origin we want. The difference is added back to all objects immediately	*/ 
	/* under the project node */ 
	vector3d p2w ( m_registration.translation() );
	vector3d adj = origin - p2w;

	m_registration.translation( origin );
	m_projectBounds.translate( adj );

	for (int i=0; i<numObjects(); i++)
	{	
		Object3D *obj = const_cast<Object3D*>(object(i));
		obj->registration().translate( -adj );
		obj->projectBounds().dirtyBounds();
		obj->projectBounds().computeBounds();
	}
	return true;
}
//-----------------------------------------------------------------------------
// Scene Class Management  | Static Functions
//-----------------------------------------------------------------------------
// global class store
//-----------------------------------------------------------------------------
namespace
{
	std::set<std::string> _types;
	std::vector<ptds::FileType> _importfiletypes;
	std::vector<ptds::FileType> _exportfiletypes;
	
	std::vector<const wchar_t*> _importfiledatatypes;
	std::vector<const wchar_t*> _exportfiledatatypes;

	typedef std::map<std::wstring, SceneClassManager*> CLASSMANAGERS;
	CLASSMANAGERS _classmanagers;
}
//-----------------------------------------------------------------------------
// SceneClassManager | Constructor 
//-----------------------------------------------------------------------------
// Group3D is used to track references only and calculate bounds etc
// Important: This is not a node in the project tree
//-----------------------------------------------------------------------------
SceneClassManager::SceneClassManager(const wchar_t*id, const wchar_t *typeDesc) : Group3D(id, 0)
{
	PTTRACE("SceneClassManager::SceneClassManager");

	_classmanagers.insert(CLASSMANAGERS::value_type(typeDesc, this));
}
SceneClassManager* SceneClassManager::manager(const wchar_t*typeDesc) 
{
	PTTRACE("SceneClassManager::manager");

	CLASSMANAGERS::iterator i = _classmanagers.find(typeDesc);
	if (i != _classmanagers.end()) return i->second;
	return 0;
}
//-----------------------------------------------------------------------------
// SceneClassManager | registerImportHandler 
//-----------------------------------------------------------------------------
bool SceneClassManager::registerImportHandler(const ptds::FileType &ftype, const wchar_t *typeDesc)
{
	PTTRACE("SceneClassManager::registerImportHandler");

	_importfiletypes.push_back(ftype);
	_importfiledatatypes.push_back(typeDesc);
	return true;
}
//-----------------------------------------------------------------------------
// SceneClassManager | registerExportHandler 
//-----------------------------------------------------------------------------
bool SceneClassManager::registerExportHandler(const ptds::FileType &ftype, const wchar_t *typeDesc)
{
	PTTRACE("SceneClassManager::registerExportHandler");
	_exportfiletypes.push_back(ftype);
	_exportfiledatatypes.push_back(typeDesc);
	return true;
}
//-----------------------------------------------------------------------------
// SceneClassManager | registerInfoImage 
//-----------------------------------------------------------------------------
bool SceneClassManager::registerInfoImage(const ptds::FilePath &path)
{
	/* TODO */ 
	return true;
}
static bool datatypeStrCmp(const wchar_t *a, const wchar_t *b)
{
	return (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3]);
}
//-----------------------------------------------------------------------------
// SceneClassManager | file types 
//-----------------------------------------------------------------------------
int SceneClassManager::numImportFileTypes(const wchar_t *typeDesc)
{ 
	if (!typeDesc) return static_cast<int>(_importfiletypes.size());
	
	int c= 0;

	for (int i=0; i<_importfiletypes.size(); i++)
		if (datatypeStrCmp(_importfiledatatypes[i], typeDesc)) c++;

	return c;
}
int SceneClassManager::numExportFileTypes(const wchar_t *typeDesc) 
{ 
	if (!typeDesc) return static_cast<int>(_exportfiletypes.size());

	int c= 0;

	for (int i=0; i<_exportfiletypes.size(); i++)
		if (datatypeStrCmp(_exportfiledatatypes[i], typeDesc)) c++;

	return c;
}
const ptds::FileType &SceneClassManager::importFileType(int i, const wchar_t *typeDesc)
{ 
	if (!typeDesc) return _importfiletypes[i]; 

	int t=0;
	for (int c=0; c<_importfiletypes.size(); c++)
	{
		if (datatypeStrCmp(_importfiledatatypes[c], typeDesc))
		{
			if (t == i) return _importfiletypes[c];
			t++;
		}
	}
	return NULL_FILETYPE;
}
const ptds::FileType &SceneClassManager::exportFileType(int i, const wchar_t *typeDesc)
{ 
	if (!typeDesc) return _exportfiletypes[i]; 

	int t=0;
	for (int c=0; c<_exportfiletypes.size(); c++)
	{
		if (datatypeStrCmp(_exportfiledatatypes[c], typeDesc))
		{
			if (t == i) return _exportfiletypes[c];
			t++;
		}
	}
	return NULL_FILETYPE;
}
int SceneClassManager::numClassManagers() { return static_cast<int>(_classmanagers.size()); }
int SceneClassManager::getClassManagers(SceneClassManager**managers)
{
	PTTRACE("SceneClassManager::getClassManagers");
	int c = 0;
	CLASSMANAGERS::iterator i = _classmanagers.begin();
	while (i != _classmanagers.end())
	{
		managers[c++] = i->second;
		++i;
	}
	return c;
}
void SceneClassInfo::clear()
{ 
	delete m_objectInfo; 
	m_objectInfo = 0;
	m_objectInfoTail = 0;
	m_infolevel = 0;
	m_objectInfoUpdated = true;
}
void SceneClassInfo::addInfo(const wchar_t* _name, int _key, void *_obj, short _imgID, bool _prop, const pt::DisplayInfo *_di)
{
	addInfo(new ObjectInfo(_name, _key, _obj, _imgID, _prop, _di));
}
//-----------------------------------------------------------------------------
template <class MatchTest>
static Object3D *s_findObject( Object3D *obj, MatchTest &m )
{
	if (m.test(obj)) return obj;

	if (obj->isGroup())
	{
		Group3D *gr = static_cast<Group3D*>(obj);

		for (int i=0; i<gr->numObjects(); i++)
		{
			Object3D *found = s_findObject(gr->object(i), m);
			if (found) return found;
		}
	}	
	return 0;
}
//-----------------------------------------------------------------------------
struct MatchByGuid
{
	MatchByGuid(Guid g) : guid(g) {};
	bool test(Object3D *obj)	{ return obj->objectGuid() == guid ? true : false; }
	Guid guid;
};
//-----------------------------------------------------------------------------
struct MatchByKey
{
	MatchByKey(ObjectKey g) : key(g) {};
	bool test(Object3D *obj)	{ return obj->key() == key ? true : false; }
	ObjectKey key;
};
//-----------------------------------------------------------------------------
struct MatchByName
{
	MatchByName(String g) : name(g) {};
	bool test(Object3D *obj)	{ return obj->identifier() == name ? true : false; }
	String name;
};
//-----------------------------------------------------------------------------
Object3D* SceneClassManager::findObjectByGuid( const Guid &guid )
{
	MatchByGuid m(guid);
	for (int i=0; i<numObjects(); i++)
	{
		Scene3D *sc = static_cast<Scene3D* >(object(i));
		Object3D *found = s_findObject(sc, m);
		if (found) return found;
	}
	return 0;
}
//-----------------------------------------------------------------------------
Scene3D* SceneClassManager::findSceneByName( const pt::String &name )
{
	for (int i=0; i<numObjects(); i++)
	{
		Scene3D *sc = static_cast<Scene3D* >(object(i));
		if (sc->identifier() == name) return sc;
	}
	return 0;
}
//-----------------------------------------------------------------------------
Scene3D* SceneClassManager::findSceneByFilepath( const ptds::FilePath &path)
{
	for (int i=0; i<numObjects(); i++)
	{
		Scene3D *sc = static_cast<Scene3D* >(object(i));
		if (sc->filepath() == path) return sc;
	}
	return 0;
}
//-----------------------------------------------------------------------------
pt::Object3D* SceneClassManager::findObjectByKey( const pt::ObjectKey &key )
{
	MatchByKey m(key);
	for (int i=0; i<numObjects(); i++)
	{
		Scene3D *sc = static_cast<Scene3D* >(object(i));
		Object3D *found = s_findObject(sc, m);
		if (found) return found;
	}
	return 0;
}
//-----------------------------------------------------------------------------
// Scene Class Management  | Static Functions
//-----------------------------------------------------------------------------
Scene3D* Project3D::addScene(const ptds::FilePath &path)
{
	PTTRACE("SceneClassManager::addScene(const ptds:FilePath &path)");
	
	if (wcscmp(path.path(), L"[empty]") == 0) return 0;

	Scene3D *sc = 0;
	CLASSMANAGERS::iterator i = _classmanagers.begin();
	while (i != _classmanagers.end())
	{
		sc = i->second->importScene(path);
		if (sc != 0) break;
		++i;
	};
	if (sc)
	{			
		sc->parent(this);
		m_scenes.insert(SCENEMAP::value_type(sc->identifier(), sc));
		m_scenesv.push_back(sc);

		m_localBounds.dirtyBounds();
		m_projectBounds.dirtyBounds();
	}

	return sc;
}
//-----------------------------------------------------------------------------
// Add scene loaded outside project to the project
//-----------------------------------------------------------------------------
void Project3D::addScene(Scene3D *sc)
{
	PTTRACE("SceneClassManager::addScene(Scene3D *scene)");

	for (int i=0; i<m_scenesv.size(); i++) if (sc == m_scenesv[i]) return;
		
	sc->parent(this);
	m_scenes.insert( SCENEMAP::value_type(sc->identifier(), sc) );
	m_scenesv.push_back(sc);

	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();

#ifdef FILE_TRACE
	Output *output;
	TraceOutput trace;
	output = &trace;
	Project3D::project().diagnostic( output, true );
#endif
}
//-----------------------------------------------------------------------------
/*
Scene3D* Project3D::scene(const wchar_t* id)
{
	PTTRACE("Project3D::scene");

	SCENEMAP::iterator i = m_scenes.find(id);
	return (i==m_scenes.end()) ? 0 : i->second;
}
*/
//-----------------------------------------------------------------------------
bool Project3D::isSceneValid(Scene3D *scene)
{
															// For all scenes
	for (int i=0; i < m_scenesv.size(); i++) 
	{
															// If scene found, record found
		if (m_scenesv[i] == scene)
		{
															// Return found
			return true;
		}
	}
															// Return not found
	return false;
}

//-----------------------------------------------------------------------------
bool Project3D::removeScene(Scene3D *sc)
{
	PTTRACE("Project3D::removeScene");

	if(isSceneValid(sc) == false)
		return false;

	SceneClassManager *m = SceneClassManager::manager(sc->typeDescriptor());

	if (m)
	{
		m->removeScene(sc, m->managesSceneMemory());
		if (!m->managesSceneMemory()) delete sc;
	}

	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();

	SCENEMAP::iterator i;
	SCENEMAP::iterator found = m_scenes.end();

	for(i = m_scenes.begin(); found == m_scenes.end() && i != m_scenes.end(); i++)
	{
		if(i->second == sc)
		{
			found = i;
		}
	}

	if(found != m_scenes.end())
		m_scenes.erase(found);

	updateSceneArray();

	if (numObjects() == 0)
		resetCoordinateSpace();

	//optimizeCoordinateSpace(true);
	return true;
}
//-----------------------------------------------------------------------------
// removeScene
//-----------------------------------------------------------------------------
bool Project3D::removeScene(const wchar_t* id)
{
	SCENEMAP::iterator i = m_scenes.find(id);
	if (i==m_scenes.end())
	{
		std::cout << "Scene could not be found for removal" << std::endl;	
		return false;
	}
	
	Scene3D *sc = i->second;

	return removeScene(sc);
}
//-----------------------------------------------------------------------------
// remove All Scenes
//-----------------------------------------------------------------------------
void Project3D::removeAllScenes()
{
	PTTRACE("Project3D::removeAllScenes");

	/* destructor of sceneclassmanager has already been called at this point */ 
	/* class manager should delete scene data */ 
	for (int i=0; i<m_scenesv.size(); i++)
	{
		Scene3D *sc = m_scenesv[i];
		SceneClassManager *m = SceneClassManager::manager(sc->typeDescriptor());
		if (m) m->removeScene(sc, m->managesSceneMemory());
		if (m && !m->managesSceneMemory()) delete sc;
	}
	m_localBounds.clearBounds();
	m_projectBounds.clearBounds();
	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();
	
	m_registration.identity();
	m_scenesv.clear();
	m_scenes.clear();
}
//-----------------------------------------------------------------------------
// Copy Scenes into vector for indexed access
//-----------------------------------------------------------------------------
void Project3D::updateSceneArray()
{
	m_scenesv.clear();
	SCENEMAP::iterator i = m_scenes.begin();
	while (i!=m_scenes.end()) { m_scenesv.push_back(i->second); ++i;}
}
//-----------------------------------------------------------------------------
// Basic commands
//-----------------------------------------------------------------------------
bool Object3D::invoke(wchar_t*command, void *data)
{
	if (wcscmp(command, L"show")==0) m_displayInfo.show();
	else if (wcscmp(command, L"hide")==0) m_displayInfo.hide();
	else return false;
	return true;
}
//-----------------------------------------------------------------------------
// The Project
//-----------------------------------------------------------------------------
namespace { Project3D *_project; }

bool initializeSceneGraphProject(void)
{
	if(_project == NULL)
	{
		_project = new Project3D;
	}

	return _project != NULL;
}


Project3D &Project3D::project(){ return *_project; }
void Project3D::initialize() {};
void Project3D::destroy() { /*removeAllScenes();*/ };


#ifdef HAVE_OPENGL
void Project3D::drawGL(uint32 m, int millisecs, const ptgl::Viewport *viewport)
{
	CLASSMANAGERS::iterator i = _classmanagers.begin();

	/* this is a hack */ 
	/* drawing must be carefuly ordered */ 
	i = _classmanagers.find(L"Point Cloud");
	if (i != _classmanagers.end() && i->second->displayInfo().visible())
			i->second->drawGL(m, millisecs, viewport);

	i = _classmanagers.find(L"3D Model");
	if (i != _classmanagers.end() && i->second->displayInfo().visible())		
		i->second->drawGL(m, millisecs, viewport);

	i = _classmanagers.find(L"Drawing");
	if (i != _classmanagers.end() && i->second->displayInfo().visible())
		i->second->drawGL(m, millisecs, viewport);

	i = _classmanagers.find(L"Note");
	if (i != _classmanagers.end() && i->second->displayInfo().visible())
		i->second->drawGL(m, millisecs, viewport);

	//for (;i!=_classmanagers.end(); i++)
	//	i->second->drawGL(m, millisecs , viewport);
}
#endif

#define STRING_FROM_WCHAR(c) pt::String( Unicode2Ascii::convert(c).c_str() )
//-----------------------------------------------------------------------------
// Visit interface : Object
//-----------------------------------------------------------------------------
void Object::visitInterface(InterfaceVisitor *visitor) const
{
	visitor->info(pt::Property("Identifier", 0, PropertyTypeName, STRING_FROM_WCHAR(m_identifier)) );
	visitor->info(pt::Property("Class", "Internal Class Name", PropertyTypeName, String(className())) );
}
int Object::setProperty(const char *id, const Variant &v)
{
	return 0;
}
int Object::getProperty(const char *id, Variant &v)
{
	return 0;
}
//-----------------------------------------------------------------------------
// Visit interface
//-----------------------------------------------------------------------------
void Object3D::visitInterface(InterfaceVisitor *visitor) const
{
	Object::visitInterface(visitor);

	visitor->property(pt::Property("Visible", "Object Visibility", PropertyTypeSwitch, m_displayInfo.visible() ));

	if (strcmp("SceneClassManager", this->objectClass())==0) return;

	vector3d l,u,wl,wu;

	if (m_projectBounds.bounds().dx() <= 0)
	{
		visitor->info(pt::Property("Lower Bound", "Lower coordinate bound of geometry", PropertyTypeValue, String("<empty>")) );
		visitor->info(pt::Property("Upper Bound", "Upper coordinate bound of geometry", PropertyTypeValue, String("<empty>")) );
		visitor->info(pt::Property("Size", "Overall dimensions", PropertyTypeValue, pt::String("<empty>")) );	
	}
	else
	{
		l.set(	static_cast<float>(m_projectBounds.bounds().lx()),
				static_cast<float>(m_projectBounds.bounds().ly()),
				static_cast<float>(m_projectBounds.bounds().lz()) );
		u.set(	static_cast<float>(m_projectBounds.bounds().ux()),
				static_cast<float>(m_projectBounds.bounds().uy()),
				static_cast<float>(m_projectBounds.bounds().uz()) );

		Project3D::project().project2WorldSpace(l, wl);
		Project3D::project().project2WorldSpace(u, wu);

		visitor->info(pt::Property("Lower Bound", "Lower coordinate bound of geometry", PropertyTypeLocation, wl) );
		visitor->info(pt::Property("Upper Bound", "Upper coordinate bound of geometry", PropertyTypeLocation, wu) );
		
		if ((u-l).length2() > 0.0001)
			visitor->info(pt::Property("Size", "Dimensions of object geometry", PropertyTypeSize, (u-l)) );
	}
}
int Object3D::setProperty(const char *id, const Variant &v)
{
	if (id && strcmp(id, "Visible")==0)
	{
		try {
			m_displayInfo.visible(ttl::var::get<bool>(v)); return 1; }
		catch (const ttl::var::exception &) {}
		return 1;
	}
	return 0;
}
int Object3D::getProperty(const char *id, Variant &v)
{
	return 0;
}
static void tab(int i) { for (int t=0;t<i; t++) printf("    "); }
//-----------------------------------------------------------------------------
// Diagnostics
//-----------------------------------------------------------------------------
void Object3D::diagnostic(pt::Output *output, bool recursive) const
{
	/* bounds */ 
	tab(output->level());
	printf("%s\n", Unicode2Ascii::convert(identifier()).c_str());
	
	tab(output->level());
	printf("{\n");

	output->push();
		
		tab(output->level());
		printf("Parent: \n\n");

		tab(output->level());
		printf("Local Bounds\n");

		tab(output->level());
		if (!m_localBounds.isValid()) printf("Bounds Invalid (Empty) \n");
		else
		{		
			printf("%f, %f, %f\n", m_localBounds.bounds().lx(), m_localBounds.bounds().ly(), m_localBounds.bounds().lz());

			tab(output->level());
			printf("%f, %f, %f\n\n", m_localBounds.bounds().ux(), m_localBounds.bounds().uy(), m_localBounds.bounds().uz());
			
			tab(output->level());
			printf("Project Bounds\n");
			
			
			tab(output->level());
			printf("%f, %f, %f\n", m_projectBounds.bounds().lx(), m_projectBounds.bounds().ly(), m_projectBounds.bounds().lz());

			tab(output->level());
			printf("%f, %f, %f\n\n", m_projectBounds.bounds().ux(), m_projectBounds.bounds().uy(), m_projectBounds.bounds().uz());
		}
		tab(output->level());
		printf("Transform\n");

		tab(output->level());
		printf("%f\t%f\t%f\t%f\n", 
			m_registration.matrix()(0,0), m_registration.matrix()(1,0), m_registration.matrix()(2,0), m_registration.matrix()(3,0));

		tab(output->level());
		printf("%f\t%f\t%f\t%f\n", 
			m_registration.matrix()(0,1), m_registration.matrix()(1,1), m_registration.matrix()(2,1), m_registration.matrix()(3,1));

		tab(output->level());
		printf("%f\t%f\t%f\t%f\n", 
			m_registration.matrix()(0,2), m_registration.matrix()(1,2), m_registration.matrix()(2,2), m_registration.matrix()(3,2));

		tab(output->level());
		printf("%f\t%f\t%f\t%f\n\n", 
			m_registration.matrix()(0,3), m_registration.matrix()(1,3), m_registration.matrix()(2,3), m_registration.matrix()(3,3));

		tab(output->level());
		printf("%s", (m_displayInfo.visible() ? "Visible\n" : "Hidden\n"));
	
	output->pop();

	tab(output->level());
	printf("}\n");
}
//-----------------------------------------------------------------------------
// Diagnostics
//-----------------------------------------------------------------------------
void Group3D::diagnostic(Output*output, bool recursive) const
{
	Object3D::diagnostic(output, false);
	output->push();
	for (int i=0; i<numObjects();i++)
	{
		const Object3D*obj = object(i);
		obj->diagnostic(output, recursive);
	}
	output->pop();
}
//-----------------------------------------------------------------------------
// Diagnostics
//-----------------------------------------------------------------------------
void Project3D::diagnostic(Output*output, bool recursive) const
{
	Object3D::diagnostic(output, false);
	if (recursive)
	{
		output->push();
		for (int i=0; i<numObjects();i++)
		{
			const Object3D* sc = object(i);
			sc->diagnostic(output, recursive);
		}
		output->pop();
	}
}
//-----------------------------------------------------------------------------
// Output
//-----------------------------------------------------------------------------

void CDECL_ATTRIBUTE Output::outputLinef(const char* fmt ...) const
{
	tab();
	va_list arg_list;
	va_start(arg_list, fmt);
	std::cout << (format(fmt, arg_list)).c_str() << std::endl;
	va_end(arg_list);			
}
void CDECL_ATTRIBUTE Output::outputf(const char* fmt ...) const
{
	tab();

	va_list arg_list;
	va_start(arg_list, fmt);
	std::cout << (format(fmt, arg_list)).c_str();
	va_end(arg_list);			
}
void Output::tab() const
{
	for (int i=0; i<_level; i++) std::cout << "\t";
}

void CDECL_ATTRIBUTE TraceOutput::outputLinef(const char* fmt ...) const
{
	tab();
	va_list arg_list;
	va_start(arg_list, fmt);
	PTTRACEOUT << (format(fmt, arg_list)).c_str() << "\n";
	
	va_end(arg_list);			
}
void CDECL_ATTRIBUTE TraceOutput::outputf(const char* fmt ...) const
{
	tab();

	va_list arg_list;
	va_start(arg_list, fmt);
	PTTRACEOUT << (format(fmt, arg_list)).c_str();
	va_end(arg_list);			
}
void TraceOutput::tab() const
{
	for (int i=0; i<_level; i++) PTTRACEOUT << "\t";
}