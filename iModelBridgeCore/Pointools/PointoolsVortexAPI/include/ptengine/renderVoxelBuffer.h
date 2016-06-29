#pragma once

#include <ptengine/renderPointsBuffer.h>

namespace pointsengine
{

//-------------------------------------------------------------------------
// VoxelInPlaceBuffer
//
// Sets up pointers based on voxel channels, no copy
// Used for shader based rendering
//-------------------------------------------------------------------------
class VoxelInPlaceBuffer : public PointsBufferI
{

public:
	VoxelInPlaceBuffer();
	virtual ~VoxelInPlaceBuffer();

	int					addPoints(	const pcloud::Voxel *voxel, int fromPoint, 
									int numPoints, SelectedPointsBuffer *selected=0 );

	void				clear();
	void				reset();

	int					setBufferSize( int newSize );

	bool				isFull() const;
	bool				isEmpty() const;
	int					getNumPoints() const;

	void				setRequiredBuffers( uint requiredBuffers );
	virtual uint		getAvailableBuffers() const;

	virtual const void	*getBufferPtr(BufferValueType bufferType) const;
	pcloud::DataType	getBufferType(BufferValueType bufferType) const;	
	int					getBufferMultiple(BufferValueType bufferType) const;	
	const double		*getBufferTransform4x4(BufferValueType bufferType) const;	

	const pcloud::Voxel *voxel() const							{ return m_voxel; }

	const ubyte			*baseColor() const;
	float				baseColorAlpha() const;

private:
	bool				tryLock();
	void				doLock();
	void				unlock();

	pcloud::Voxel		*m_voxel;
	int					m_numPoints;
	int					m_fromPoint;

	// unused - ubyte				m_selCol[4];
};
	//-------------------------------------------------------------------------
	// SelectedPointsBuffer
	//
	// Selected points only need point position
	//-------------------------------------------------------------------------
	class SelectedVoxelPointsBuffer : public VoxelInPlaceBuffer
	{	
	public:
		SelectedVoxelPointsBuffer( const ubyte *selCol );
		virtual ~SelectedVoxelPointsBuffer();

		const ubyte		*baseColor() const;
		uint			getAvailableBuffers() const { return Buffer_Pos; };
		const void		*getBufferPtr(BufferValueType bufferType) const;
		
	private:
		const ubyte		*m_selectionCol;
	};
}