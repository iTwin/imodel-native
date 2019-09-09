/******************************************************************************

Pointools Vortex API Examples

ChannelTestRenderer.cpp

Provides query based rendering for channels example 
demonstrates channel querying

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/

#include "ChannelTestRender.h"
//-----------------------------------------------------------------------------
ChannelTestRenderer::ChannelTestRenderer(int buffersize) 
: QueryRender(buffersize)
//-----------------------------------------------------------------------------
{

}

//-----------------------------------------------------------------------------
void ChannelTestRenderer::modifyPointsWithChannelValues()
//-----------------------------------------------------------------------------
{
	int num_points = m_queryBuffer.numPntsInQueryIteration();
	
	float *points = const_cast<float*>( m_queryBuffer.getPointsBuffer() );		// its safe, but don't do this at home :)
	PTubyte *rgbBuffer = const_cast<PTubyte*>( m_queryBuffer.getRGBBuffer() );

	const float *offsetVals = reinterpret_cast<const float*>(m_queryBuffer.getChannelBuffer(0));
	const PTubyte *cols = reinterpret_cast<const PTubyte*>(m_queryBuffer.getChannelBuffer(1));
	
	for (int i=0; i<num_points*3; i+=3)
	{
		points[i] += m_mixer * offsetVals[i];
		points[i+1] += m_mixer * offsetVals[i+1];
		points[i+2] += m_mixer * offsetVals[i+2];

		// if these are adjusted values, not default white
		if (cols[i] != 255 && cols[i+1] != 255 && cols[i+2] != 255)
		{
			rgbBuffer[i] = (PTubyte) (((1.0f-m_mixer) * rgbBuffer[i] ) + m_mixer * cols[i]);
			rgbBuffer[i+1] = (PTubyte) (((1.0f-m_mixer) * rgbBuffer[i+1] ) + m_mixer * cols[i+1]);
			rgbBuffer[i+2] = (PTubyte) (((1.0f-m_mixer) * rgbBuffer[i+2] ) + m_mixer * cols[i+2]);
		}
	}
}
//-----------------------------------------------------------------------------
bool ChannelTestRenderer::drawPointClouds(bool dynamic, bool clearFrame)
//-----------------------------------------------------------------------------
{
	if (!m_channels[0] || !m_channels[1])
	{		
		QueryRender::drawPointClouds(dynamic, clearFrame);
		return true;
	}

	prepareRender(clearFrame, dynamic);

	{
		ptResetQuery( m_queryHandle );

		pt::PerformanceTimer timer;
		timer.start();

		while (m_queryBuffer.executeQuery( m_queryHandle, m_channels, 2 ))
		{
		
			// modify the position and color using the channel data
			// yes this is slow, but its for example only
			// this would be better performed in a shader
			modifyPointsWithChannelValues();

			// do the actual draw using a GL vertex array
			glPointSize( dynamic ? 2.0f : 1.0f);

			glEnable( GL_COLOR_MATERIAL );
			glDisable( GL_LIGHTING );

			glEnableClientState( GL_VERTEX_ARRAY );
			glEnableClientState( GL_COLOR_ARRAY );

			// point positions
			glVertexPointer( 3, GL_FLOAT, 0, m_queryBuffer.getPointsBuffer() );

			// point color
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, m_queryBuffer.getRGBBuffer() );

			glDrawArrays( GL_POINTS, 0, m_queryBuffer.numPntsInQueryIteration() );

			glDisableClientState( GL_VERTEX_ARRAY );
			glDisableClientState( GL_COLOR_ARRAY );

			// prevent long frame render times - can complete in another iteration
			if (timer.millisecs() > 500) break;
		}
		ptEndDrawFrameMetrics();
		return !(m_queryBuffer.isBufferFull());	// is view complete? if buffer is full it means there is another iteration to come
	}
	return true;
}
//-----------------------------------------------------------------------------
void ChannelTestRenderer::setChannels( PThandle offset, PThandle col )
//-----------------------------------------------------------------------------
{
	m_channels[0] = offset;
	m_channels[1] = col;
}
//-----------------------------------------------------------------------------
void ChannelTestRenderer::setMixer( float mixer )
//-----------------------------------------------------------------------------
{
	m_mixer = mixer;
}

