/*----------------------------------------------------------------------*/ 
/* PointCloud.cpp														*/ 
/* Point Cloud Implementation file										*/ 
/*----------------------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004											*/   
/*----------------------------------------------------------------------*/ 
/* Written by Faraz Ravi												*/ 
/*----------------------------------------------------------------------*/ 
#include <map>

#include <pt\os.h>

#include <ptcloud2\pointcloud.h>
#include <ptcloud2\buildindex.h>

#include <pt\project.h>

#include <iostream>

//#define DEBUGGING_PCLOUD
#ifdef POINTOOLS_API_INCLUDE
#undef debugAssertM
#define debugAssertM(a,b) // if(!a) std::cout << "assertion failed!!" << std::endl
#endif
using namespace pcloud;

//---------------------------------------------------------------
// Construction  
//-----------------------------------------------------------------------------
PointCloud::PointCloud(const wchar_t*id, const Scene *scene, float compressionTolerance) 
	: pt::Object3D(id, 0)
{
	_root = 0;
	_guid = 0;
	_ibound = 0;
	_jbound = 0; 
	_numpoints = 0;
	_guid = 0;
	_compressionTolerance = compressionTolerance;
	_scene = scene;
	_pointSize=0; // use global setting
	_baseColour[0] = 255;
	_baseColour[1] = 255;
	_baseColour[2] = 255;
	memset(_channels, 0, sizeof(bool)*MAX_CHANNELS);
	_filepointer = 0;
}
//---------------------------------------------------------------
// Destruction 
//---------------------------------------------------------------
PointCloud::~PointCloud()
{
	try
	{
	delete _root;
	}
	catch (...){}
}
//---------------------------------------------------------------
// Set Root 
//--------------------------------------------------------------
void PointCloud::setRoot(Node*n)
{
	if (_root && _root != n) delete _root;
	_root = n;
	unsigned int i;
	for (i=0; i<_voxels.size(); i++) _voxels[i]->pointCloud(this);
	
	m_localBounds.dirtyBounds();
	m_projectBounds.dirtyBounds();

	/* compute memory used */ 
	_memused = 0;
	_numpoints = 0;
	for (i=0; i<_voxels.size(); i++)
	{
		_voxels[i]->_index = i;

		for (int j=0; j<MAX_CHANNELS; j++)
		{
			const DataChannel *dc = _voxels[i]->channel(j);
			if (dc)
			{
				_channels[j] = true;
				_memused += _voxels[i]->fullPointCount() * dc->typesize() * dc->multiple();
			}
		}
		_numpoints += _voxels[i]->fullPointCount();
	}
	int nodes = _root->countNodes() - _voxels.size();
	_memused += sizeof(Node) * nodes;
	_memused += (sizeof(Voxel) + sizeof(void*) ) * _voxels.size();
}
//
const Voxel* PointCloud::findContainingVoxel(const pt::vector3d &seek_pnt, pt::CoordinateSpace cs) const
{
#ifndef POINTOOLS_POD_API
	mmatrix4d m;
	m_registration.compileMatrix(m, pt::ProjectSpace);
	m >>= _userTransform.matrix();

	pt::vector3d pnt(seek_pnt);

	if (cs == pt::ProjectSpace)
	{
		m.invert();
		m.vec3_multiply_mat4(seek_pnt, pnt);
	}	

	int  point_index;
	const Voxel* vox=0;
	double mindist = -1;
	double d;
	pt::vector3d check, nr;

	/*check world space bounds*/ 
	pt::BoundingBoxD bndsd(m_localBounds.bounds().ux(), m_localBounds.bounds().lx(), 
		m_localBounds.bounds().uy(), m_localBounds.bounds().ly(), 
		m_localBounds.bounds().uz(), m_localBounds.bounds().lz());

	if (!bndsd.inBounds(pnt))
	{
		return 0;
	}

	const Node *leaf = root()->findContainingLeaf(pnt);

	if (leaf)
	{
		return static_cast<const Voxel*>(leaf);;
	}
#endif
	return 0;
		
}
//---------------------------------------------------------------
// compute Bounds
//---------------------------------------------------------------
double PointCloud::findNearestPoint(const pt::vector3d &_pnt, pt::vector3d &nearest, pt::CoordinateSpace cs) const
{
#ifndef POINTOOLS_POD_API
	mmatrix4d m;
	m_registration.compileMatrix(m, pt::ProjectSpace);
	m >>= _userTransform.matrix();

	pt::vector3d pnt(_pnt);

	if (cs == pt::ProjectSpace)
	{
		m.invert();
		m.vec3_multiply_mat4(_pnt, pnt);
	}
	else
	{
		debugAssertM(cs == pt::LocalSpace, "This Coordinate space is not supported");
		if (cs != pt::LocalSpace) return -1;
		pnt = _pnt;
	}	

	int  point_index;
	const Voxel* vox=0;
	double mindist = -1;
	double d;
	pt::vector3d check, nr;

	/*check world space bounds*/ 
	pt::BoundingBoxD bndsd(m_localBounds.bounds().ux(), m_localBounds.bounds().lx(), 
		m_localBounds.bounds().uy(), m_localBounds.bounds().ly(), 
		m_localBounds.bounds().uz(), m_localBounds.bounds().lz());
	if (!bndsd.inBounds(pnt))
	{
		return -1;
	}

	Node *leaf = root()->findContainingLeaf(pnt);

	if (leaf)
	{
		Voxel* V = static_cast<Voxel*>(leaf);
		boost::mutex::scoped_lock lk(V->mutex(), boost::try_to_lock);

		if (lk.owns_lock())
		{
			int idx = V->findNearestPoint(pnt, check, d);

			if (idx >= 0 && (mindist < 0 || d < mindist))
			{
				nr = check;
				point_index = idx;				
				mindist = d;
				vox = V;
			}
			
		}
	}
	if (mindist >= 0)
	{		
		m.invert();
		m.vec3_multiply_mat4(pt::vector3d(nr), nearest);
	}
	return mindist;
#else
	return 0;
#endif
}
// returns distance along ray ie t
double PointCloud::findIntersectingPoint(const pt::Rayf &ray, pt::vector3d &nearest, float tolerance, pt::CoordinateSpace cs) const
{
	if (!_root) return -1;

#ifndef POINTOOLS_POD_API
	pt::Rayf xray = ray;

	mmatrix4d m;
	m_registration.compileMatrix(m, pt::ProjectSpace);
	m >>= _userTransform.matrix();

	if (cs == pt::ProjectSpace)
	{
		m.vec3_multiply_mat4f(ray.direction, xray.direction);
		m.invert();
		m.vec3_multiply_mat4f(ray.origin, xray.origin);
	}
	else
	{
		debugAssertM(cs == pt::LocalSpace, "This Coordinate space is not supported");
		if (cs != pt::LocalSpace) return -1;
	}
	Node* leaf = _root->findIntersectingLeaf(xray);

	if (leaf)
	{
		Voxel* vox = static_cast<Voxel*>(leaf);
		boost::mutex::scoped_lock lk(vox->mutex(), boost::try_to_lock);

		double t;
		int pnt = vox->intersectRay(xray, t, tolerance);
		if (pnt >= 0)
		{
			m.invert();
			DataChannel* dc = vox->channel(pcloud::PCloud_Geometry);
		
			if (dc->typesize() == sizeof(float))
				dc->getval(nearest, pnt);
			else 
			{	
				pt::vec3<short> snr;
				dc->getval(snr, pnt);

				nearest.set(snr.x,snr.y,snr.z);
				nearest *= dc->scaler();
				nearest += dc->offset();

				return t;
			}

		}
	}
#endif
	return -1;
}
//---------------------------------------------------------------
// compute Bounds
//---------------------------------------------------------------
void PointCloud::_computeBounds()
{
	//assert(_root);

	if (!_root) return;

	if (m_localBounds.areBoundsDirty()) 
	{
		m_localBounds.undirtyBounds();
		m_localBounds.clearBounds();
		pt::BoundingBox bb;
		_root->getBounds(bb);
		m_localBounds.expandBounds(bb);
	}
	if (m_projectBounds.areBoundsDirty())
	{
		/* if there is a user transform, force recompute */ 
		bool hasXform = ((userTransformationMatrix() != mmatrix4d::identity()) || transform().isDirty()) ? true : false;
		if (hasXform)
			for (int i=0; i<_voxels.size(); i++) _voxels[i]->flag( ExtentsDirty, true );

		_root->computeExtents();
		m_projectBounds.undirtyBounds();
		m_projectBounds.clearBounds();
		m_projectBounds.expandBounds(_root->extents());
		/* voxel extents are in world space not project */ 
		m_projectBounds.transformBounds(pt::Project3D::project().registration().invMatrix());
	}
}
//---------------------------------------------------------------
// build pointcloud from raw data
//---------------------------------------------------------------
void PointCloud::buildFromRaw(int n, float *geom, ubyte *rgb, short *intensity, short *norms)
{
	if (!geom) return;
	std::vector<BuildIndex::ChannelData*> channels;
	
	channels.push_back(new BuildIndex::ChannelData(PCloud_Geometry, geom, n));
	if (rgb) channels.push_back(new BuildIndex::ChannelData(PCloud_RGB, rgb, n));
	if (intensity) channels.push_back(new BuildIndex::ChannelData(PCloud_Intensity, intensity, n));
	if (norms) channels.push_back(new BuildIndex::ChannelData(PCloud_Normal, norms, n));

	setRoot(BuildIndex::buildUniformPointsTree(channels,_voxels));
}
//---------------------------------------------------------------
// Clear data 
//---------------------------------------------------------------
void PointCloud::clearData()
{
	for (int i=0; i<_voxels.size(); i++)
		_voxels[i]->clearChannels();
}
//---------------------------------------------------------------
// Inititalize 
//---------------------------------------------------------------
bool PointCloud::initialize()
{
	randomizeData();
	return true;
}
//---------------------------------------------------------------
// randomize datachannel data order 
//---------------------------------------------------------------
void PointCloud::randomizeData()
{
	for (int i=0; i<_voxels.size(); i++)
		_voxels[i]->initializeChannels();
}
//---------------------------------------------------------------
// generate deterministic GUID
//---------------------------------------------------------------
bool PointCloud::setGuid( PointCloudGUID pcguid )
{	
	if (!_guid) _guid = pcguid;
	else return false;
	return true;
}
//---------------------------------------------------------------
// generate deterministic GUID
//---------------------------------------------------------------
void PointCloud::generateGuid()
{	
	/* this is just random - because we need to be able to distiguish between duplicate clouds*/ 
	GUID g;
	CoCreateGuid( &g );
	
	memcpy(&_guid, &g, sizeof(_guid));
}
//---------------------------------------------------------------
// Object guid, based on the cloud guid
// Note that many places in Vortex use the __int64 guid when writing,
// e.g. UserChannel persistance, this object guid should not be
// used in these places, it is currently only used with the Clash trees.
//---------------------------------------------------------------
pt::Guid PointCloud::objectGuid() const
{	
	pt::Guid g(guid());
	g.setPart2(0); // Cloud has 0 here, Scene has 1 (@see Scene::objectGuid())
	return g;	
}
//---------------------------------------------------------------
//
//---------------------------------------------------------------
void PointCloud::visitInterface(pt::InterfaceVisitor *visitor) const
{
	Object3D::visitInterface(visitor);

	visitor->property(pt::Property("Offset", "Offset to position", pt::PropertyTypeLocation, _userTransform.translation() ));	
	visitor->property(pt::Property("Rotation", "Euler rotation relative to Parent", pt::PropertyTypeAngle, _userTransform.eulers() ));	
	visitor->property(pt::Property("Scale", "Scaling", pt::PropertyTypeAngle, _userTransform.scale() ));	
	visitor->property(pt::Property("PointSize", "Independent point size, 0 = use global setting", pt::PropertyTypeSwitch, _pointSize));

	unsigned int col = RGB(_baseColour[0], _baseColour[1], _baseColour[2]);
	visitor->property(pt::Property("BaseColour", "Underlaying colour. Default is white", pt::PropertyTypeColor, col));
}
int PointCloud::setProperty(const char *id, const pt::Variant &v)
{
	if (pt::Object3D::setProperty(id, v) == 1) return 1;
	
	pt::vector3d trans(_userTransform.translation());
	pt::vector3d eulers(_userTransform.eulers());
	pt::vector3d scale(_userTransform.scale());
	
	int retval = 1;
	unsigned int col;

	if (_setProperty(id, "Offset", v, trans))
		_userTransform.setTranslation(trans);
	else if (_setProperty(id, "Rotation", v, eulers))
		_userTransform.setEulers(eulers);
	else if (_setProperty(id, "Scale", v, scale))
		_userTransform.setScale(scale);
	else if (_setProperty(id, "BaseColour", v, col))
	{
		_baseColour[0] = GetRValue(col);
		_baseColour[1] = GetGValue(col);
		_baseColour[2] = GetBValue(col);
	}
	else retval = 0;
	
	_setProperty(id, "PointSize", v, _pointSize);
	if (_pointSize > 10.0f) _pointSize = 10.0f;
	if (_pointSize < 0.5f) _pointSize = -.5f;

	_userTransform.updateMatrix();

	return retval;	
}
int PointCloud::getProperty(const char *id, pt::Variant &v)
{
	if (pt::Object::getProperty(id, v) ==1 ) return 1;

	int retval = 0;

	if (strcmp(id, "Offset") == 0)
	{
		v = pt::Variant(_userTransform.translation());
		retval = 1;
	}
	else if (strcmp(id, "Rotation")==0)
	{
		v = pt::Variant(_userTransform.eulers());
		retval = 1;
	}
	else if (strcmp(id, "Scale") == 0)
	{
		v = pt::Variant(_userTransform.scale());
		retval = 1;
	}
	else if (strcmp(id, "PointSize") == 0)
	{
		v = pt::Variant(_pointSize);
		retval = 1;
	}
	return retval;
}
void PointCloud::pushUserTransformation() const
{ 
#ifndef POINTOOLS_POD_API
	bool dirty = _userTransform.isDirty();
	const_cast<UserTransform*>(&_userTransform)->pushGL(); 

	if (dirty != _userTransform.isDirty())
	{
		m_localBounds.dirtyBounds();
		m_projectBounds.dirtyBounds();

		PointCloud *thispc = const_cast<PointCloud*>(this);
		thispc->parent()->localBounds().dirtyBounds();
		thispc->parent()->projectBounds().dirtyBounds();


		/*fast version hacks the extents, if there's no rotation or scaling */ 
		if (_userTransform.eulers().is_zero())
		{
			for (int i=0; i<_voxels.size(); i++)
			{
				Voxel *vox = const_cast<Voxel*>(_voxels[i]);
				
			//	boost::mutex::scoped_lock vlock(vox->mutex());
			
				vox->_worldExtents = vox->_origextents;
				pt::vector3d trans(_userTransform.translation());
				vox->_worldExtents.translateBy(trans);
				vox->flag( ExtentsDirty, false );
			}
		}
		else
		{
			for (int i=0; i<_voxels.size(); i++)
			{
				Voxel *vox = const_cast<Voxel*>(_voxels[i]);
				vox->flag( ExtentsDirty, true );
				vox->computeExtents();
			}
		}
	}
#endif
}

void UserTransform::pushGL()
{
#ifndef POINTOOLS_POD_API

	if (_flags & 1)
	{
		/* this will not work if we are not using openGL */ 
		if (!(_flags & 2)) /* use transforms directly */ 
		{
			glPushMatrix();
			glLoadIdentity();

			glTranslated(_translation.x, _translation.y, _translation.z);
			glRotated(_eulers.z, 0, 0, 1);
			glRotated(_eulers.y, 0, 1, 0);
			glRotated(_eulers.x, 1, 0, 0);
			glScaled(_scale.x, _scale.y, _scale.z);
			_matrix.loadGLmodel();

			glPopMatrix();
		}
		_flags &= ~1;		
	}
	glPushMatrix();

	if (_flags & 2)
	{
		/* openGL uses col matrix */ 
		_matrix.transpose();
		glMultMatrixd(_matrix.data());
		_matrix.transpose();
	}
	else
	{
		glTranslated(_translation.x, _translation.y, _translation.z);
		glRotated(_eulers.z, 0, 0, 1);
		glRotated(_eulers.y, 0, 1, 0);
		glRotated(_eulers.x, 1, 0, 0);
		glScaled(_scale.x, _scale.y, _scale.z);
		_cmpmatrix.loadGLmodel();
	}
#endif
}
void UserTransform::updateMatrix()
{
	if (_flags & 1)
	{
		if (!(_flags & 2)) /* use transforms directly */ 
		{
			_matrix = mmatrix4d::scale( &_scale.x )
				>> mmatrix4d::rotation( _eulers.x, _eulers.y, _eulers.z) 
				>> mmatrix4d::translation( &_translation.x );
		}
		_flags &= ~1;	

		/* todo :compiled matrix */
	}
}
