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
	m_validPnts = ptGetQueryPointsd( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, m_layerBuffer, m_classificationBuffer );
	return m_validPnts;
}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<float>::executeQuery( PThandle query )
//-----------------------------------------------------------------------------
{
	m_validPnts = ptGetQueryPointsf( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, m_layerBuffer, m_classificationBuffer );
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

		m_validPnts = ptGetDetailedQueryPointsd( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, 0,
			m_layerBuffer, m_classificationBuffer, 1, &channel, m_channelBuffer );

		return m_validPnts;
	}
	else return executeQuery( query );
}
//-----------------------------------------------------------------------------
template<>
int64_t	QueryBuffer<float>::countPointsInQuery( PThandle query )
{
	ptResetQuery( query );
	
	int64_t count = 0;
	m_validPnts = 0;

	do
	{
		m_validPnts = ptGetQueryPointsf( query, m_size, m_pointsBuffer, 0, 0, 0, 0 );
		count += m_validPnts;
	}
	while (m_validPnts);

	return count;

}
//-----------------------------------------------------------------------------
int64_t		QueryBuffer<float>::computeQueryBoundingBox( PThandle query, PTdouble *lower3, PTdouble *upper3 )	// returns point count
{
	int64_t count = 0;
	m_validPnts = 0;

	ptResetQuery( query );

	upper3[0] = upper3[1] = upper3[2] = -1e16;
	lower3[0] = lower3[1] = lower3[2] = 1e16;

	do
	{
		m_validPnts = ptGetQueryPointsf( query, m_size, m_pointsBuffer, 0, 0, 0, 0 );
		count += m_validPnts;

		for (int i=0; i<m_validPnts;i++)
		{
			for (int j=0;j<3;j++)
			{
				if (lower3[j] > m_pointsBuffer[i*3+j]) lower3[j] = m_pointsBuffer[i*3+j];
				if (upper3[j] < m_pointsBuffer[i*3+j]) upper3[j] = m_pointsBuffer[i*3+j];
			}
		}
	}
	while (m_validPnts);

	return count;
}
//-----------------------------------------------------------------------------
template<>
int64_t	QueryBuffer<double>::countPointsInQuery( PThandle query )
{
	int64_t count = 0;
	m_validPnts = 0;

	ptResetQuery( query );

	do
	{
		m_validPnts = ptGetQueryPointsd( query, m_size, m_pointsBuffer, 0, 0, 0, 0 );
		count += m_validPnts;
	}
	while (m_validPnts);

	return count;

}
//-----------------------------------------------------------------------------
template<>
int QueryBuffer<float>::executeQuery( PThandle query, PThandle channel )
//-----------------------------------------------------------------------------
{
	if (channel)
	{
		allocateChannelBuffer( channel, 0 );
	
		m_validPnts = ptGetDetailedQueryPointsf( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, 0,
			m_layerBuffer, m_classificationBuffer, 1, &channel, m_channelBuffer );
	
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

		m_validPnts = ptGetDetailedQueryPointsd( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, 0,
			m_layerBuffer, m_classificationBuffer, numChannels, channels, m_channelBuffer );

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

		m_validPnts = ptGetDetailedQueryPointsf( query, m_size, m_pointsBuffer, m_rgbBuffer, m_intensityBuffer, 0,
			m_layerBuffer, m_classificationBuffer, numChannels, channels, m_channelBuffer );
	
		return m_validPnts;
	}
	else return executeQuery( query );
}
