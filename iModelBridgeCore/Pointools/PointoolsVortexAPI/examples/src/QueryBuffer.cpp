/******************************************************************************

Pointools Vortex API Examples

QueryBuffer.cpp

Encapsulates query based point extraction from Vortex into buffers

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

//----------------------------------------------------------------------------

*******************************************************************************/
#include "QueryBuffer.h"

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
	if (channel)
	{
		allocateChannelBuffer( channel, 0 );

		m_validPnts = ptGetDetailedQueryPointsd( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer,
			0, 0, 0, 1, &channel, m_channelBuffer );

		return m_validPnts;
	}
	else return executeQuery( query );
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<float>::executeQuery( PThandle query, PThandle channel )
//-----------------------------------------------------------------------------
{
	if (channel)
	{
		allocateChannelBuffer( channel, 0 );
	
		m_validPnts = ptGetDetailedQueryPointsf( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer,
			0, 0, 0, 1, &channel, m_channelBuffer );
	
		return m_validPnts;
	}
	else return executeQuery( query );
}

//-----------------------------------------------------------------------------
template<>
int QueryBuffer<double>::executeQuery( PThandle query, PThandle *channels, int numChannels )
//-----------------------------------------------------------------------------
{
	if (channels)
	{
		for (int i=0; i<numChannels; i++)
		{
			if (channels[i])
				allocateChannelBuffer( channels[i], i );
		}

		m_validPnts = ptGetDetailedQueryPointsd( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer,
			0, 0, 0, numChannels, channels, m_channelBuffer );

		return m_validPnts;
	}
	else return executeQuery( query );
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<float>::executeQuery( PThandle query, PThandle *channels, int numChannels )
//-----------------------------------------------------------------------------
{
	if (channels)
	{
		for (int i=0; i<numChannels; i++)
		{
			if (channels[i])
				allocateChannelBuffer( channels[i], i );
		}

		m_validPnts = ptGetDetailedQueryPointsf( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer,
			0, 0, 0, numChannels, channels, m_channelBuffer );
	
		return m_validPnts;
	}
	else return executeQuery( query );
}
