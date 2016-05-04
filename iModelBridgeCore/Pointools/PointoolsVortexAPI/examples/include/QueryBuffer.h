/******************************************************************************

Pointools Vortex API Examples

QueryBuffer.h

Encapsulates query based point extraction from Vortex into buffers

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_QUERY_BUFFER_H_
#define POINTOOLS_EXAMPLE_APP_QUERY_BUFFER_H_

#include <PTAPI/PointoolsVortexAPI_import.h>
#include <memory>

#define QUERY_BUFFER_MAX_CHANNELS 8

template< typename T >
class QueryBuffer
{
public:
	QueryBuffer( int size ) 
		: m_size(size), m_pointsBuffer(0), m_rgbBuffer(0), 
		m_intensityBuffer(0), m_classificationBuffer(0),
		m_validPnts(0), m_layerBuffer(0)
	{
		for (int i=0; i<QUERY_BUFFER_MAX_CHANNELS;i++)
			m_channelBuffer[i]=0;
	}
	~QueryBuffer()
	{
		clear();
	}
	bool					initialize	( bool needRGB=false, bool needIntensity=false, bool needClassification=false, bool needLayers=false );
	void					clear();
	int						executeQuery( PThandle query );	
	int						executeQuery( PThandle query, PThandle channel );	
	int						executeQuery( PThandle query, PThandle *channels, int numChannels );	

	inline const T			*getPoint( int index ) const				{ return &m_pointsBuffer[index*3]; }
	inline const T			*getPointsBuffer()	const					{ return m_pointsBuffer; }
	inline const PTubyte	*getRGBBuffer()	const						{ return m_rgbBuffer; }
	inline const PTubyte	*getClassificationBuffer() const			{ return m_classificationBuffer; }
	inline const PTubyte	*getLayerBuffer() const						{ return m_layerBuffer; }
	inline PTubyte			*getRGB( int index )						{ return &m_rgbBuffer[index*3]; }
	
	void					*getChannelBuffer( int channelIndex=0 ) const	{ return m_channelBuffer[ channelIndex ]; }

	template <class C>		bool getChannelValue( int channelIndex, int pointIndex, C &val ) const;
	template <class C>		bool setChannelValue( int channelIndex, int pointIndex, const C &val );
	
	int						numPntsInQueryIteration() const				{ return m_validPnts; }
	bool					isBufferFull() const						{ return m_validPnts == m_size ? true : false; }

	int64_t					countPointsInQuery( PThandle query );	

	int64_t					computeQueryBoundingBox( PThandle query, PTdouble *lower3, PTdouble *upper3 );	// returns point count

private:
	bool					allocateChannelBuffer( PThandle channel, int channelIndex );

	T						*m_pointsBuffer;
	PTubyte					*m_rgbBuffer;
	PTshort					*m_intensityBuffer;
	PTubyte					*m_classificationBuffer;
	PTubyte					*m_layerBuffer;
	PTvoid					*m_channelBuffer[QUERY_BUFFER_MAX_CHANNELS];

	int						m_size;
	int						m_validPnts;

	PThandle 				m_channel[QUERY_BUFFER_MAX_CHANNELS];

};
typedef QueryBuffer<float>	QueryBufferf;
typedef QueryBuffer<double> QueryBufferd;

//-----------------------------------------------------------------------------
template< typename T >
bool QueryBuffer<T>::initialize( bool needRGB/*=false*/, 
								bool needIntensity/*=false */, 
								bool needClassification/*=false*/,
								bool needLayers/*=false*/)
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

		if (needClassification && !m_classificationBuffer)
			m_classificationBuffer = new PTubyte[m_size];

		if (needLayers && !m_layerBuffer)
			m_layerBuffer = new PTubyte[m_size];

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
bool QueryBuffer<T>::allocateChannelBuffer( PThandle channel, int channelIndex )
//-----------------------------------------------------------------------------
{
	if (m_channelBuffer[channelIndex])
	{
		if (m_channel[channelIndex] == channel) return true;
		else delete [] m_channelBuffer[channelIndex];
		m_channelBuffer[channelIndex] = 0;
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
		m_channelBuffer[channelIndex] = new PTubyte[bytes];
		m_channel[channelIndex] = channel;
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
	
	if (m_classificationBuffer)	delete [] m_classificationBuffer;
		m_classificationBuffer=0;
	
	if (m_layerBuffer)	delete [] m_layerBuffer;
		m_layerBuffer=0;

	for (int i=0; i<QUERY_BUFFER_MAX_CHANNELS;i++)
	{
		if (m_channelBuffer[i])	delete [] m_channelBuffer[i];
		m_channelBuffer[i]=0;
	}
}
//-----------------------------------------------------------------------------
template< typename T >
template< class C >	bool QueryBuffer<T>::getChannelValue( int channelIndex, int pointIndex, C &val ) const
//-----------------------------------------------------------------------------
{
	if (m_channelBuffer)
	{
		memcpy(&val, &(reinterpret_cast<C*>(m_channelBuffer[channelIndex])[pointIndex]), sizeof(C));
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
template< typename T >
template< class C >	bool QueryBuffer<T>::setChannelValue( int channelIndex, int pointIndex, const C &val )
//-----------------------------------------------------------------------------
{
	if (m_channelBuffer)
	{
		memcpy( &(reinterpret_cast<C*>(m_channelBuffer[channelIndex])[pointIndex]), &val, sizeof(C) );
		return true;
	}
	return false;
}
#endif