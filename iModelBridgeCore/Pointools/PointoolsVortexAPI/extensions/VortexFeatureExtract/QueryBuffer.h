/******************************************************************************

Pointools Vortex API Examples

QueryBuffer.h

Encapsulates query based point extraction from Vortex into buffers

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_QUERY_BUFFER_H_
#define POINTOOLS_EXAMPLE_APP_QUERY_BUFFER_H_

#include "Includes.h"
#include <memory>

namespace vortex
{

template< typename T >
class QueryBuffer
{
public:
	QueryBuffer( int size ) 
		: m_size(size), m_pointsBuffer(0), m_rgbBuffer(0), m_intensityBuffer(0), 
		m_validPnts(0), m_channelBuffer(0)
	{}
	~QueryBuffer()
	{
		clear();
	}
	bool					initialize	( bool needRGB=false, bool needIntensity=false );
	void					clear();

	int						executeQuery( void );	
	int						executeQueryChannel( PThandle channel );	
	int						executeQuery( PThandle query );	
	int						executeQuery( PThandle query, PThandle channel );	

	inline const T			*getPoint	( int index ) const				{ return &m_pointsBuffer[index*3]; }
	inline const T			*getPointsBuffer()	const					{ return m_pointsBuffer; }
	inline const PTubyte	*getRGBBuffer()	const						{ return m_rgbBuffer; }
	inline PTubyte			*getRGB		( int index ) const				{ return &m_pointsBuffer[index*3]; }

	template <class C>		bool getChannelValue( int index, C &val ) const;
	template <class C>		bool setChannelValue( int index, const C &val );

	int						numPntsInQueryIteration() const				{ return m_validPnts; }
	bool					isBufferFull() const						{ return m_validPnts == m_size ? true : false; }

	// where QueryBuffer is passed as an argument and pre-loaded with the query
	void					setQuery( PThandle query )					{ m_query = query; }
	bool					resetQuery()								{ return ptResetQuery( m_query ); }

	PThandle				query() const								{ return m_query; }

private:
	bool					allocateChannelBuffer( PThandle channel );

	T			*m_pointsBuffer;
	PTubyte		*m_rgbBuffer;
	PTshort		*m_intensityBuffer;
	PTvoid		*m_channelBuffer;

	int			m_size;
	int			m_validPnts;
	PThandle 	m_channel;
	PThandle	m_query;

};

typedef QueryBuffer<float>	QueryBufferf;
typedef QueryBuffer<double> QueryBufferd;

//-----------------------------------------------------------------------------
template< typename T >
bool QueryBuffer<T>::initialize( bool needRGB/*=false*/, bool needIntensity/*=false */ )
//-----------------------------------------------------------------------------
{
	try
	{
		if (!m_pointsBuffer) 
			m_pointsBuffer = new T[m_size*3];

		if (needRGB && !m_rgbBuffer) 
			m_rgbBuffer = new PTubyte[m_size*3];

		if (needIntensity && !m_intensityBuffer) 
			m_intensityBuffer = new PTshort[m_size*3];

		return true;
	}
	catch(std::bad_alloc)
	{
		clear();
		return false;
	}
}
//-----------------------------------------------------------------------------
template< typename T >
bool QueryBuffer<T>::allocateChannelBuffer( PThandle channel )
//-----------------------------------------------------------------------------
{
	if (m_channelBuffer)
	{
		if (m_channel == channel) return true;
		else delete [] m_channelBuffer;
		m_channelBuffer = 0;
	}

	wchar_t name[128];
	PTenum typesize;
	PTuint multiple;
	PTubyte *defaultVal[128];
	PTuint flags;

	ptGetChannelInfo( channel, name, typesize, multiple, defaultVal, flags);

	int bytes = typesize * multiple * m_size;

	try
	{
		m_channelBuffer = new PTubyte[bytes];
		m_channel = channel;
		return true;
	}
	catch (std::bad_alloc)
	{
		return false;
	}
}
//-----------------------------------------------------------------------------
template< typename T >
void QueryBuffer<T>::clear()
//-----------------------------------------------------------------------------
{
	if (m_pointsBuffer)	delete [] m_pointsBuffer;
	m_pointsBuffer=0;
	if (m_rgbBuffer)	delete [] m_rgbBuffer;
	m_rgbBuffer=0;
	if (m_intensityBuffer)	delete [] m_intensityBuffer;
	m_intensityBuffer=0;
	if (m_channelBuffer)	delete [] m_channelBuffer;
	m_channelBuffer=0;
}
//-----------------------------------------------------------------------------
template< typename T >
template< class C >	bool QueryBuffer<T>::getChannelValue( int index, C &val ) const
//-----------------------------------------------------------------------------
{
	if (m_channelBuffer)
	{
		memcpy(&val, &(reinterpret_cast<C*>(m_channelBuffer)[index]), sizeof(C));
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
template< typename T >
template< class C >	bool QueryBuffer<T>::setChannelValue( int index, const C &val )
//-----------------------------------------------------------------------------
{
	if (m_channelBuffer)
	{
		memcpy( &(reinterpret_cast<C*>(m_channelBuffer)[index]), &val, sizeof(C) );
		return true;
	}
	return false;
}
}
#endif