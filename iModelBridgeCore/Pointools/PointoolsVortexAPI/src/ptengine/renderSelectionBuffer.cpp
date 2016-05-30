#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderPointsBuffer.h>
#include <ptedit/edit.h>

using namespace pointsengine;
using namespace pcloud;
using namespace pt;

#define DEFAULT_BUFFER_SIZE 1000000

using namespace pointsengine;

SelectedPointsBuffer::SelectedPointsBuffer( const ubyte *selCol ) : m_selectionCol( selCol )
{
	m_points = 0;
	m_numPoints =0 ;
	m_bufferSize = 0;
}
SelectedPointsBuffer::~SelectedPointsBuffer()
{
	clear();
}
int SelectedPointsBuffer::setBufferSize( int newSize )
{
	clear();
	m_bufferSize = newSize;
	try
	{
		m_points = new pt::vector3[ m_bufferSize ];
	}
	catch (std::bad_alloc e)
	{
		m_bufferSize = 0;
	}

	return m_bufferSize;
}
void	SelectedPointsBuffer::setBaseColour( const ubyte * selCol )
{
	m_selectionCol = selCol;
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void SelectedPointsBuffer::reset()
{
	m_numPoints = 0;
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void SelectedPointsBuffer::clear()
{
	if (m_points)
		delete [] m_points;
	m_points = 0;
	m_numPoints = 0;
	m_bufferSize = 0;
}
/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool SelectedPointsBuffer::isFull() const
{
	return (m_numPoints >= m_bufferSize) ? true : false;
}

/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool SelectedPointsBuffer::isEmpty() const
{
	return (!m_numPoints) ? true : false;
}

/*****************************************************************************/
/**
* @brief
* @return int
*/
/*****************************************************************************/
int SelectedPointsBuffer::getNumPoints() const
{
	return m_numPoints;
}

/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return const void			*
*/
/*****************************************************************************/
const void			*SelectedPointsBuffer::getBufferPtr( BufferValueType bufferType ) const
{
	return (bufferType == Buffer_Pos) ? m_points : 0;
}

/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return pcloud::DataType
*/
/*****************************************************************************/
pcloud::DataType	SelectedPointsBuffer::getBufferType( BufferValueType bufferType ) const
{
	return pcloud::Float32;
}

/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return int
*/
/*****************************************************************************/
int					SelectedPointsBuffer::getBufferMultiple( BufferValueType bufferType ) const
{
	return 3;
}

/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return const double		*
*/
/*****************************************************************************/
const double		*SelectedPointsBuffer::getBufferTransform4x4( BufferValueType bufferType ) const
{
	static mmatrix4d id = mmatrix4d::identity();	// points are pre-transformed
	return id.data();						// just return identity
}
/*****************************************************************************/
/**
* @brief
* @param pt
* @return int
*/
/*****************************************************************************/
int SelectedPointsBuffer::addPoint( const pt::vector3 &pt )
{
	if (!m_points) setBufferSize( DEFAULT_BUFFER_SIZE );
	if (!m_points) return 0;

	if (m_numPoints < m_bufferSize)
	{
		m_points[ m_numPoints ] = pt;
		++m_numPoints;
	}
	return m_numPoints;
}

/*****************************************************************************/
/**
* @brief
* @param voxel
* @param fromPoint
* @param numPoints
* @param selected
* @return int
*/
/*****************************************************************************/
int SelectedPointsBuffer::addPoints( const pcloud::Voxel *voxel, int fromPoint, int numPoints, SelectedPointsBuffer *selected/*=0 */ )
{
	if (!m_points)
	{
		setBufferSize(DEFAULT_BUFFER_SIZE);
	}
	if (!numPoints) return 0;

	int ptsAvailable = voxel->channel(PCloud_Geometry)->size() - fromPoint;

	if (numPoints > ptsAvailable) numPoints = ptsAvailable;
	if (numPoints < 0) return 0;

	const pt::vector3s		*geom16 = 0;
	const pt::vector3		*geom = 0;

	const PointCloud *pc = voxel->pointCloud();
	const_cast<pt::Transform*>(&pc->registration())->compileMatrix();
	mmatrix4d mat = pc->registration().cmpMatrix();
	mat >>= pc->userTransformationMatrix();

	vector3 geom_scaler, geom_offset;
	geom_scaler.set( voxel->channel(PCloud_Geometry)->scaler() );
	geom_offset.set( voxel->channel(PCloud_Geometry)->offset() );

	const pcloud::DataChannel *chGeom =	voxel->channel(PCloud_Geometry);

	// setup pointers to channels
	if (chGeom->storeType() == pcloud::Short16)
		chGeom->getConstPtr(&geom16, fromPoint);
	else	chGeom->getConstPtr(&geom, fromPoint);

	//final values
	vector3 g;

	//TODO: validate numPoints and start position	
	int startPoint = m_numPoints;

	for (int p=0; p<numPoints; p++)
	{
		if (m_numPoints == m_bufferSize) 
			break;

		// DO POSITION
		if (geom16)
		{
			g.set(geom16->x, geom16->y, geom16->z);
			++geom16;
		}
		else if (geom)
		{
			g = *geom;
			++geom;
		}
		g *= geom_scaler;
		g += geom_offset;

		mat.vec3_multiply_mat4f(g, m_points[ m_numPoints ]);

		++m_numPoints;
	}
	return (m_numPoints - startPoint);		//number of points actually added
}

bool SelectedPointsBuffer::tryLock()
{
	return true;
}

void SelectedPointsBuffer::doLock()
{
}

void SelectedPointsBuffer::unlock()
{
}
const ubyte *SelectedPointsBuffer::baseColor() const
{
	return m_selectionCol;
}
float	SelectedPointsBuffer::baseColorAlpha() const
{
	return 1.0f;
}
