#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderVoxelBuffer.h>

using namespace pointsengine;
using namespace pcloud;

/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
VoxelInPlaceBuffer::VoxelInPlaceBuffer()
{
	m_numPoints = 0;
	m_voxel = 0;
}

/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
VoxelInPlaceBuffer::~VoxelInPlaceBuffer()
{

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
int		VoxelInPlaceBuffer::addPoints(	const pcloud::Voxel *voxel, int fromPoint, 
			int numPoints, SelectedPointsBuffer *selected )
{
	m_voxel = const_cast<Voxel*>(voxel);

	//if (m_voxel->mutex().try_lock())
	//{
	//	m_voxel->mutex().unlock();
	//	assert(0);						// should not be able to lock, should be already locked
	//	return 0;
	//}
	m_fromPoint = fromPoint;
	m_numPoints = numPoints;

	return 0;
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void	VoxelInPlaceBuffer::clear()
{
	m_voxel = 0;
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void	VoxelInPlaceBuffer::reset()
{
	m_voxel = 0;
}

/*****************************************************************************/
/**
* @brief
* @param newSize
* @return int
*/
/*****************************************************************************/
int		VoxelInPlaceBuffer::setBufferSize( int newSize )
{
	return (m_voxel) ? static_cast<int>(m_voxel->size()) : 0;		// has no effect;
}

/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool	VoxelInPlaceBuffer::isFull() const
{
	return m_voxel ? true : false;				// full if has voxel;
}
/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool	VoxelInPlaceBuffer::isEmpty() const
{
	return m_voxel ? false : true;				// full if has voxel;
}
/*****************************************************************************/
/**
* @brief
* @return int
*/
/*****************************************************************************/
int		VoxelInPlaceBuffer::getNumPoints() const
{
	// validated number of points
	if (!m_voxel) return 0;

	uint maxNum = m_numPoints;
	
	// would not work for delayed channel loading
	for (int i=1; i<PCloud_NumChannelTypes; i++)
	{
		const DataChannel *dc = m_voxel->channel(i);
		if (dc && dc->size() < maxNum )
			maxNum = dc->size();
	}
	uint npe = m_voxel->numPointsEdited();

	if (npe && npe < maxNum) 
		maxNum = npe;

	return maxNum;		// m_voxel must be valid
}
/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return const void			*
*/
/*****************************************************************************/
const void			*VoxelInPlaceBuffer::getBufferPtr(BufferValueType bufferType) const
{
	if (!m_voxel) return 0;

	switch ( bufferType ) 
	{
	case Buffer_Pos:
		return m_voxel->channel( PCloud_Geometry )->data();

	case Buffer_RGB:
		if (m_voxel->channel(PCloud_RGB))
			return m_voxel->channel( PCloud_RGB )->data();
		break;

	case Buffer_TexCoord0:
		if (m_voxel->channel(PCloud_Intensity))
			return m_voxel->channel( PCloud_Intensity )->data();
		break;

	case Buffer_Normal:
		if (m_voxel->channel(PCloud_Normal))
			return m_voxel->channel( PCloud_Normal )->data();
		break;

	case Buffer_Layers:
		if (m_voxel->channel(PCloud_Filter))
			return m_voxel->channel( PCloud_Filter )->data();
		break;

	default:
		return 0;
	}
	return 0;
}
/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return pcloud::DataType
*/
/*****************************************************************************/
pcloud::DataType	VoxelInPlaceBuffer::getBufferType(BufferValueType bufferType) const
{
	if (!m_voxel) return pcloud::NullType;

	switch ( bufferType )
	{
	case Buffer_Pos:
		return (m_voxel->channel( PCloud_Geometry )->typesize() == 2) ?
			Short16 : Float32;
	
	case Buffer_RGB:
		return UByte8;

	case Buffer_TexCoord0:
		return Short16;

	case Buffer_Normal:
		return Short16;

	default:
		return NullType;
	}
	return NullType;
}
/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return int
*/
/*****************************************************************************/
int					VoxelInPlaceBuffer::getBufferMultiple(BufferValueType bufferType) const
{
	switch ( bufferType )
	{
	case Buffer_Pos:
		return 3;

	case Buffer_RGB:
		return 3;

	case Buffer_TexCoord0:
		return 1;

	case Buffer_Normal:
		return 3;

	default:
		return 0;
	}
}
/*****************************************************************************/
/**
* @brief
* @param bufferType
* @return const double		*
*/
/*****************************************************************************/
const double		*VoxelInPlaceBuffer::getBufferTransform4x4(BufferValueType bufferType) const
{
	if (bufferType == Buffer_Pos)
	{
		static mmatrix4d mat;
		mat = mmatrix4d::identity();
		
		/* data channel stuff */ 
		pcloud::DataChannel *data = m_voxel->channel(pcloud::PCloud_Geometry);
		double scale[] = { data->scaler()[0], data->scaler()[1], data->scaler()[2], 1.0 };
		mat >>= mmatrix4d::scale( scale );
		mat(3,0) = data->offset()[0];
		mat(3,1) = data->offset()[1];
		mat(3,2) = data->offset()[2];
		mat(3,3) = 1.0;

		/* registration */ 
		const PointCloud *pc = m_voxel->pointCloud();
		const_cast<pt::Transform*>(&pc->registration())->compileMatrix();
		mmatrix4d reg = pc->registration().cmpMatrix();
		reg >>= pc->userTransformationMatrix();

		mat >>= reg;

		return mat.data();
	}
	return 0;
}
void VoxelInPlaceBuffer::setRequiredBuffers( uint requiredBuffers )
{

}
uint VoxelInPlaceBuffer::getAvailableBuffers() const 
{
	uint channels = Buffer_Pos;
	if (!m_voxel) return 0;

	channels |= m_voxel->channel( pcloud::PCloud_Intensity ) ? Buffer_TexCoord0 : 0;
	channels |= m_voxel->channel( pcloud::PCloud_RGB ) ? Buffer_RGB : 0;
	channels |= m_voxel->channel( pcloud::PCloud_Normal ) ? Buffer_Normal : 0;
	channels |= m_voxel->channel( pcloud::PCloud_Filter ) ? Buffer_Layers : 0;

	return channels;
}

bool	VoxelInPlaceBuffer::tryLock()
{ 
	if (m_voxel)
	{
		try
		{
			return m_voxel->mutex().try_lock();
		}
		catch (...){}
	}
	return false;

}
void	VoxelInPlaceBuffer::doLock()
{
	if (m_voxel) 
	{
		try 
		{
			m_voxel->mutex().lock();
		}
		catch(...){}
	}
	
}
void	VoxelInPlaceBuffer::unlock()
{
	if (m_voxel)
	{
		try 
		{
			m_voxel->mutex().unlock();
		}
		catch (...) {}	// will throw if not locked
	}
}
const ubyte *VoxelInPlaceBuffer::baseColor() const
{
	static ubyte col[] = { 255, 255, 255, 255 };	// use the voxels layer colour
	
	return  m_voxel->pointCloud()->isOverriderColorEnabled() ?  m_voxel->pointCloud()->overrideColor() : col;
}
float VoxelInPlaceBuffer::baseColorAlpha() const
{
	return m_voxel->pointCloud() ? (float)m_voxel->pointCloud()->overrideColor()[3]/255 : 1.0f;
}
SelectedVoxelPointsBuffer::SelectedVoxelPointsBuffer( const ubyte *selCol ) : m_selectionCol( selCol )
{
}
SelectedVoxelPointsBuffer::~SelectedVoxelPointsBuffer()
{
}
const ubyte *SelectedVoxelPointsBuffer::baseColor() const
{
	return	m_selectionCol;
}
const void	*SelectedVoxelPointsBuffer::getBufferPtr(BufferValueType bufferType) const
{
	if (bufferType==Buffer_Pos) 
		return VoxelInPlaceBuffer::getBufferPtr( bufferType );
	return 0;
}
