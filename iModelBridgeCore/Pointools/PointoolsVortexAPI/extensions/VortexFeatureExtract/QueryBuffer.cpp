/******************************************************************************

Pointools Vortex API Examples

QueryBuffer.cpp

Encapsulates query based point extraction from Vortex into buffers

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#include "QueryBuffer.h"

namespace vortex
{
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<float>::executeQuery( void )
//-----------------------------------------------------------------------------
{
	if (m_query)
	{
		return executeQuery(m_query);
	}
	return 0;
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<double>::executeQuery( void )
//-----------------------------------------------------------------------------
{
	if (m_query)
	{
		return executeQuery(m_query);
	}
	return 0;
}
//-----------------------------------------------------------------------------
template<>
int vortex::QueryBuffer<float>::executeQueryChannel( PThandle channel )
//-----------------------------------------------------------------------------
{
	if (m_query)
	{
		if (channel)
			return executeQuery(m_query, channel);
		else
			return executeQuery(m_query);
	}
	return 0;
}
//-----------------------------------------------------------------------------
template<>
int vortex::QueryBuffer<double>::executeQueryChannel( PThandle channel )
//-----------------------------------------------------------------------------
{
	if (m_query)
	{
		return executeQuery(m_query, channel);
	}
	return 0;
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<double>::executeQuery( PThandle query )
//-----------------------------------------------------------------------------
{
	m_validPnts = ptGetQueryPointsd( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, 0, 0 );
	return m_validPnts;
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<float>::executeQuery( PThandle query )
//-----------------------------------------------------------------------------
{
	m_validPnts = ptGetQueryPointsf( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, 0, 0 );
	return m_validPnts;
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<double>::executeQuery( PThandle query, PThandle channel )
//-----------------------------------------------------------------------------
{
	allocateChannelBuffer( channel );

	m_validPnts = ptGetDetailedQueryPointsd( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer,
		0, 0, 0, 1, &channel, &m_channelBuffer );
	return m_validPnts;
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<float>::executeQuery( PThandle query, PThandle channel )
//-----------------------------------------------------------------------------
{
	allocateChannelBuffer( channel );

	m_validPnts = ptGetDetailedQueryPointsf( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer,
		0, 0, 0, 1, &channel, &m_channelBuffer );

	return m_validPnts;
} 
}