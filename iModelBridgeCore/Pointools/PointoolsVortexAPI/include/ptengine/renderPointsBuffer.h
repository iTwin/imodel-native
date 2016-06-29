#pragma once

#include <ptcloud2/voxel.h>
#include <ptcloud2/pointcloud.h>
#include <ptcloud2/datachannel.h>

namespace pointsengine
{
	class RenderSettings;
	class SelectedPointsBuffer;

	enum BufferValueType
	{
		Buffer_None			= 0,
		Buffer_Pos			= 0x1,
		Buffer_RGB			= 0x2,
		Buffer_Normal		= 0x4,
		Buffer_TexCoord0	= 0x8,
		Buffer_TexCoord1	= 0x10,
		Buffer_TexCoord2	= 0x20,
		Buffer_TexCoord3	= 0x40,
		Buffer_Layers		= 0x80,
		Buffer_UserArray	= 0x100
	};

	//-------------------------------------------------------------------------
	// PointsBufferI
	//
	// RenderingAPI (GL/DX/SW) independent points buffer
	//-------------------------------------------------------------------------
	class PointsBufferI
	{
	public:
		virtual	~PointsBufferI() { ; }

		virtual int					addPoints(	const pcloud::Voxel *voxel, int fromPoint, 
												int numPoints, SelectedPointsBuffer *selected )=0;

		virtual void				clear()=0;
		virtual void				reset()=0;

		virtual bool				isFull() const=0;
		virtual bool				isEmpty() const=0;
		virtual int					getNumPoints() const=0;

		virtual void				setRequiredBuffers( uint requiredBuffers )=0;
		virtual uint				getAvailableBuffers() const=0;

		virtual const void			*getBufferPtr(BufferValueType bufferType) const=0;
		virtual pcloud::DataType	getBufferType(BufferValueType bufferType) const=0;	
		virtual int					getBufferMultiple(BufferValueType bufferType) const=0;	
		virtual const double		*getBufferTransform4x4(BufferValueType bufferType) const=0;	

		virtual const ubyte			*baseColor() const=0;
		virtual float				baseColorAlpha() const=0;

	private:
		friend	class				BufferScopedTryLock;
		friend	class				BufferScopedLock;

		virtual	bool				tryLock()=0;	// attempts
		virtual void				doLock()=0;	// blocking
		virtual void				unlock()=0;  

	};
	//-------------------------------------------------------------------------
	// SelectedPointsBuffer
	//
	// Selected points only need point position
	//-------------------------------------------------------------------------
	class SelectedPointsBuffer : public PointsBufferI
	{	
	public:
		SelectedPointsBuffer( const ubyte *selCol=0 );
		virtual ~SelectedPointsBuffer();

		int					addPoints( const pcloud::Voxel *voxel, int fromPoint, int numPoints, SelectedPointsBuffer *selected=0 );
		int					addPoint( const pt::vector3 &pt );

		void				clear();
		void				reset();

		int					setBufferSize( int newSize );

		bool				isFull() const;
		bool				isEmpty() const;
		int					getNumPoints() const;
		void				setRequiredBuffers( uint requiredBuffers ) {};
		uint				getAvailableBuffers() const { return Buffer_Pos; };

		void				setBaseColour( const ubyte * selCol );
		const ubyte			*baseColor() const;
		float				baseColorAlpha() const;

		const void			*getBufferPtr(BufferValueType bufferType) const;
		pcloud::DataType	getBufferType(BufferValueType bufferType) const;	
		int					getBufferMultiple(BufferValueType bufferType) const;	
		const double		*getBufferTransform4x4(BufferValueType bufferType) const;

	private:
		bool				tryLock();
		void				doLock();
		void				unlock();

		int					m_bufferSize;
		int					m_numPoints;
		pt::vector3			*m_points;

		const ubyte			*m_selectionCol;
	};
	//-------------------------------------------------------------------------
	// Points Buffer
	//
	// Copies points into a new buffer
	// Used for fixed pipeline rendering and aggregating small voxels
	//-------------------------------------------------------------------------
	class PointsBuffer : public PointsBufferI
	{
	public:
		PointsBuffer();
		virtual ~PointsBuffer();

		int					addPoints( const pcloud::Voxel *voxel, int fromPoint, int numPoints, SelectedPointsBuffer *selected );
		
		void				clear();
		void				reset();

		int					setBufferSize( int newSize );

		bool				isFull() const;
		bool				isEmpty() const;
		int					getNumPoints() const;
		void				setRequiredBuffers( uint requiredBuffers );
		uint				getAvailableBuffers() const;

		const void			*getBufferPtr(BufferValueType bufferType) const;
		pcloud::DataType	getBufferType(BufferValueType bufferType) const;	
		int					getBufferMultiple(BufferValueType bufferType) const;	
		const double		*getBufferTransform4x4(BufferValueType bufferType) const;	

		const ubyte			*baseColor() const;
		float				baseColorAlpha() const;

	private:
		bool				allocateBufferChannel( BufferValueType buffer );
		bool				tryLock();
		void				doLock();
		void				unlock();

		int					m_bufferSize;
		int					m_numPoints;
		pt::vector3			*m_points;
		ubyte				*m_rgb;
		pt::vector3			*m_normal;
		short				*m_texCoord0;
		float				*m_texCoord1;
		uint				m_requiredBuffers;
	};

//-------------------------------------------------------------------------
	// BufferScopedLock mutex lock
	//-------------------------------------------------------------------------
	class BufferScopedTryLock
	{
	public:
		BufferScopedTryLock(PointsBufferI *buffer) : m_buffer(buffer)	
		{	
			m_isLocked = buffer->tryLock(); 
		}
		~BufferScopedTryLock()					{	if (m_isLocked) m_buffer->unlock(); }
		bool			isLocked()	const		{ return m_isLocked; }
	private:
		PointsBufferI	*m_buffer;
		bool			m_isLocked;
	};
	//-------------------------------------------------------------------------
	// BufferScopedLock blocking mutex lock
	//-------------------------------------------------------------------------
	class BufferScopedLock
	{
	public:
		BufferScopedLock(PointsBufferI *buffer) : m_buffer(buffer)	
		{	
			buffer->doLock(); 
		}
		~BufferScopedLock()						{	m_buffer->unlock(); }
	private:
		PointsBufferI	*m_buffer;
		// unused - bool			m_isLocked;
	};
}