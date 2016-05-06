#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderPointsBuffer.h>
#include <ptedit/edit.h>

#include <ptengine/pointLayers.h>
#include <ptengine/engine.h>
#include <ptengine/userChannels.h>

using namespace pointsengine;
using namespace pcloud;
using namespace pt;

#define DEFAULT_BUFFER_SIZE 1000000

/*****************************************************************************/
/**
* @brief			PointsBuffer constructor, does not allocate
*/
/*****************************************************************************/
PointsBuffer::PointsBuffer()
{	
	m_numPoints = 0;
	m_normal = 0;
	m_points = 0;
	m_rgb = 0;
	m_texCoord0 = 0;
	m_texCoord1 = 0;
	m_bufferSize = 0;

	m_requiredBuffers = Buffer_Pos | Buffer_RGB | Buffer_Normal | Buffer_TexCoord0 | Buffer_TexCoord1;
}

/*****************************************************************************/
/**
* @brief			PointsBuffer destructor
*/
/*****************************************************************************/
PointsBuffer::~PointsBuffer()
{
	clear();
}

/*****************************************************************************/
/**
* @brief			Sets the buffer size and allocates
* @param newSize	Desired new size of buffer
* @return int		Actual size of buffer, as number of points allocated
*/
/*****************************************************************************/
int PointsBuffer::setBufferSize( int newSize )
{
	if (m_numPoints == newSize) return m_numPoints;

	int numPoints = newSize;
	
	while (!m_points)				// attempt to allocate a buffer, decrease buffer size on failure
	{
		bool hasRgb = m_rgb ? true : false;
		bool hasTex0 = m_texCoord0 ? true : false;
		bool hasTex1 = m_texCoord1 ? true : false;
		bool hasNormal = m_normal ? true : false;

		clear();

		try
		{
			m_points = new vector3[ numPoints ];
			
			if (hasRgb)		m_rgb = new ubyte[ numPoints * 3];
			if (hasTex0)	m_texCoord0 = new short[ numPoints ];
			if (hasTex1)	m_texCoord1 = new float[ numPoints ];
			if (hasNormal)	m_normal = new vector3[ numPoints ];
		}
		catch (...)
		{
			if (m_points)		delete [] m_points;
			if (m_rgb)			delete [] m_rgb;
			if (m_texCoord0)	delete [] m_texCoord0;
			if (m_texCoord1)	delete [] m_texCoord1;
			if (m_normal)		delete [] m_normal;

			m_rgb = 0;
			m_texCoord0 = 0;
			m_texCoord1 = 0;
			m_normal = 0;
			m_points = 0;

            numPoints = static_cast<int>(numPoints * 0.75);
		}
	}
	m_bufferSize = numPoints;
	m_numPoints = 0;			// ie valid points
	return numPoints;
}

/*****************************************************************************/
/**
* @brief
* @param buffer
* @return bool
*/
/*****************************************************************************/
bool PointsBuffer::allocateBufferChannel( BufferValueType bufferType )
{
	try
	{

	switch( bufferType )
	{
	case Buffer_RGB:		
		if (!m_rgb) 
		{
			m_rgb = new ubyte[ m_bufferSize * 3];
			for (int i=0; i<m_bufferSize*3; i++)	//set rgb to white
				m_rgb[i] = 255;
		}
		break;

	case Buffer_Normal:		
		if (!m_normal) m_normal = new vector3[ m_bufferSize ];
		break;

	case Buffer_TexCoord0:	
		if (!m_texCoord0) m_texCoord0 = new short[ m_bufferSize ];
		break;

	case Buffer_TexCoord1:	
		if (!m_texCoord1) m_texCoord1 = new float[ m_bufferSize ];
		break;
	}
	}
	catch(...)
	{
		return false;
	}
	return true;
}

/*****************************************************************************/
/**
* @brief			Deallocates the buffer and sets back to construction state
*/
/*****************************************************************************/
void PointsBuffer::clear()
{
	if (m_points) delete [] m_points;
	if (m_rgb)	delete [] m_rgb;
	if (m_normal) delete [] m_normal;
	if (m_texCoord0) delete [] m_texCoord0;
	if (m_texCoord1) delete [] m_texCoord1;

	m_bufferSize = 0;
	m_normal = 0;
	m_points = 0;
	m_rgb = 0;
	m_texCoord0 = 0;
	m_texCoord1 = 0;
}

/*****************************************************************************/
/**
* @brief			Resets buffer counters, does not deallocate
*/
/*****************************************************************************/
void PointsBuffer::reset()
{
	if (m_rgb)
	{
		for (int i=0; i<m_numPoints*3; i++)	//set rgb to white
			m_rgb[i] = 255;
	}

	m_numPoints = 0;
}

/*****************************************************************************/
/**
* @brief			Adds a voxels points to the buffer
* @param voxel		The voxel to extract points from
* @param transform	Indicates if the points should be transformed into project space
* @param fromPoint	The point (index) from which the points should be extracted
* @param numPoints	The number of points to be extracted
* @return int		The number of points actually added to the buffer
*/
/*****************************************************************************/
int PointsBuffer::addPoints( const pcloud::Voxel *voxel, int fromPoint, int numPoints, 
							SelectedPointsBuffer *selected )
{
	if (!m_points)
	{
		setBufferSize(DEFAULT_BUFFER_SIZE);
	}
	if (!numPoints) return 0;

	const pt::vec3<ubyte>	*rgb = 0;
	const short				*intensity = 0;
	const vector3s			*normal = 0;
	const ubyte				*filter = 0;
	const pt::vector3s		*geom16 = 0;
	const pt::vector3		*geom = 0;

	const PointCloud *pc = voxel->pointCloud();
	const_cast<Transform*>(&pc->registration())->compileMatrix();
	mmatrix4d mat = pc->registration().cmpMatrix();
	mat >>= pc->userTransformationMatrix();

	vector3 geom_scaler, geom_offset;
	geom_scaler.set( voxel->channel(PCloud_Geometry)->scaler() );
	geom_offset.set( voxel->channel(PCloud_Geometry)->offset() );

	const pcloud::DataChannel *chGeom =	voxel->channel(PCloud_Geometry);
	const pcloud::DataChannel *chRGB =	voxel->channel(PCloud_RGB);
	const pcloud::DataChannel *chIntensity =	voxel->channel(PCloud_Intensity);
	const pcloud::DataChannel *chNormal =	voxel->channel(PCloud_Normal);
	const pcloud::DataChannel *chFilter =	voxel->channel(PCloud_Filter);

	// setup pointers to channels
	if (chGeom->storeType() == pcloud::Short16)
			chGeom->getConstPtr(&geom16, fromPoint);
	else	chGeom->getConstPtr(&geom, fromPoint);

	if (!geom && !geom16) return 0;

	// user channels if needed for rendering are added here
	UserChannel *offsetChannel = UserChannelManager::instance()->renderOffsetChannel();
	VoxelChannelData *voxelChannel = 0;
	if (offsetChannel)
	{
		voxelChannel = offsetChannel->voxelChannel( voxel );
		offsetChannel->unlock( voxelChannel, numPoints+fromPoint );
		if (!voxelChannel->getData())
		{
			offsetChannel->lock( voxelChannel );
			voxelChannel = 0;
		}
	}

	if (chRGB && m_requiredBuffers & Buffer_RGB) 			
	{
		chRGB->getConstPtr(&rgb, fromPoint);
		if (!allocateBufferChannel( Buffer_RGB )) rgb = 0;
	}
	if (chIntensity && m_requiredBuffers & Buffer_TexCoord0)	
	{
		chIntensity->getConstPtr(&intensity, fromPoint);
		if (!allocateBufferChannel( Buffer_TexCoord0 )) intensity = 0;
	}
	if (chNormal && m_requiredBuffers & Buffer_Normal)		
	{
		chNormal->getConstPtr(&normal, fromPoint);
		if (!allocateBufferChannel( Buffer_Normal )) normal =0 ;
	}
	if (chFilter)		chFilter->getConstPtr(&filter, fromPoint);

	//final values
	vector3 g;

	ubyte visLayers = thePointLayersState().pointVisLayerMask();

	//TODO: validate numPoints and start position	
	int startPoint = m_numPoints;

	vector3 offset(0,0,0);
	float blend = UserChannelManager::instance()->renderOffsetBlendValue();

	for (int p=0; p<numPoints; p++)
	{
		if (m_numPoints == m_bufferSize) 
			break;
				
		// Do FILTER
		if (filter && !(*filter & visLayers))
		{
			if (geom)			++geom;
			else if (geom16)	++geom16;
			if (rgb)			++rgb;
			if (intensity)		++intensity;
			if (normal)			++normal;
			++filter;

			continue;
		}

		// DO POSITION
		if (voxelChannel)	// offset channel
		{
			voxelChannel->getValR( p + fromPoint, offset );
			offset *= blend;
		}

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
		m_points[ m_numPoints ] += offset;

		// if selected put into selected buffer
		if (filter && *filter & SELECTED_PNT_BIT) 
		{
			selected->addPoint(m_points[ m_numPoints ]);

			if (rgb)			++rgb;
			if (intensity)		++intensity;
			if (normal)			++normal;
			++filter;

			continue;
		}

		// Do Intensity
		if (intensity && m_texCoord0)
		{
			m_texCoord0[ m_numPoints ] = (*intensity);		
			++intensity;
		}

		//Do RGB
		if (rgb && filter && m_rgb)
		{
			m_rgb[ m_numPoints *3 ] = rgb->x;
			m_rgb[ m_numPoints *3+1 ] = rgb->y;
			m_rgb[ m_numPoints *3+2 ] = rgb->z;
			++rgb;
		}

		// Normals
		if (normal && m_normal)
		{
			vector3 n( normal->x, normal->y, normal->z );
			n /= 32768.0f;
			m_normal[ m_numPoints ] =  n;
			++normal;
		}

		if (filter) ++filter;
		++m_numPoints;
	}
	// rgb copied straight in if no filter
	if (!filter && m_rgb && rgb)
	{
		memcpy( &m_rgb[ startPoint * 3], rgb, 3 * (m_numPoints - startPoint));
	}

	// lock offsetchannel again
	if (offsetChannel)	offsetChannel->lock( voxelChannel );

	return (m_numPoints - startPoint);		//number of points actually added
}

/*****************************************************************************/
/**
* @brief		
* @return bool
*/
/*****************************************************************************/
bool PointsBuffer::isFull() const
{
	return (m_numPoints >= m_bufferSize) ? true : false;
}


/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool PointsBuffer::isEmpty() const
{
	return (!m_numPoints) ? true : false;
}



/*****************************************************************************/
/**
* @brief
* @return int
*/
/*****************************************************************************/
int PointsBuffer::getNumPoints() const
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
const void			*PointsBuffer::getBufferPtr( BufferValueType bufferType ) const
{
	switch( bufferType )
	{
	case Buffer_None:		return 0;
	case Buffer_Pos:		return m_points;
	case Buffer_RGB:		return m_rgb;
	case Buffer_Normal:		return m_normal;
	case Buffer_TexCoord0:	return m_texCoord0;
	case Buffer_TexCoord1:	return m_texCoord1;
	default:				return 0;
	}
}

/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return pcloud::DataType
*/
/*****************************************************************************/
pcloud::DataType PointsBuffer::getBufferType( BufferValueType bufferType ) const
{
	switch( bufferType )
	{
	case Buffer_None:		return pcloud::NullType;
	case Buffer_Pos:		return pcloud::Float32;
	case Buffer_RGB:		return pcloud::UByte8;
	case Buffer_Normal:		return pcloud::Float32;
	case Buffer_TexCoord0:	return pcloud::Short16;
	case Buffer_TexCoord1:	return pcloud::Float32;
	default:				return pcloud::NullType;
	}
}
/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return int
*/
/*****************************************************************************/
int					PointsBuffer::getBufferMultiple(BufferValueType bufferType) const
{
	switch( bufferType )
	{
	case Buffer_None:		return 0;
	case Buffer_Pos:		return 3;
	case Buffer_RGB:		return 3;
	case Buffer_Normal:		return 3;
	case Buffer_TexCoord0:	return 1;
	case Buffer_TexCoord1:	return 1;
	default:				return 0;
	}
}
/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return int
*/
/*****************************************************************************/
const double		*PointsBuffer::getBufferTransform4x4(BufferValueType bufferType) const
{
	static mmatrix4d id = mmatrix4d::identity();	// points are pre-transformed
	return id.data();						// just return identity
}

/*****************************************************************************/
/**
* @brief	trims buffer usage to those required only
* @param requiredBuffers
* @return void
*/
/*****************************************************************************/
void PointsBuffer::setRequiredBuffers( uint requiredBuffers )
{
	m_requiredBuffers = requiredBuffers;

	if (!(m_requiredBuffers & Buffer_TexCoord0) && m_texCoord0)		// don't trim rgb buffer, needed for selection
	{
		delete [] m_texCoord0;
		m_texCoord0 = 0;
	}
	if (!(m_requiredBuffers & Buffer_RGB) && m_rgb)
	{
		delete [] m_rgb;
		m_rgb = 0;
	}
	if (!(m_requiredBuffers & Buffer_TexCoord1) && m_texCoord1)
	{
		delete [] m_texCoord1;
		m_texCoord1 = 0;
	}
	if (!(m_requiredBuffers & Buffer_Normal) && m_normal)
	{
		delete [] m_normal;
		m_normal = 0;
	}
}

uint PointsBuffer::getAvailableBuffers() const 
{
	uint channels = Buffer_Pos;

	if (m_normal)		channels |= Buffer_Normal;
	if (m_texCoord0)	channels |= Buffer_TexCoord0;
	if (m_rgb)			channels |= Buffer_RGB;
	
	return channels;
}

bool PointsBuffer::tryLock()
{
	return true;
}

void PointsBuffer::doLock()
{
}

void PointsBuffer::unlock()
{
}
const ubyte *PointsBuffer::baseColor( ) const
{
	static ubyte col[] = { 255,255,255, 255 };
	return col;
}

float	PointsBuffer::baseColorAlpha( ) const
{
	return 1.0f;
}