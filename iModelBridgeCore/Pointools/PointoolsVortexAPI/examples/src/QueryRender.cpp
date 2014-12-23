/******************************************************************************

Pointools Vortex API Examples

QueryRender.cpp

Provides query based rendering - demonstrates query capability and rendering
values from user channels

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#include "QueryRender.h"

//-----------------------------------------------------------------------------
QueryRender::QueryRender(int buffersize) : m_queryBuffer(buffersize), m_queryHandle(0)
//-----------------------------------------------------------------------------
{
	pt::PerformanceTimer::initialize();
	m_maxPoints = 0;
}
//-----------------------------------------------------------------------------
void QueryRender::createFrustumQuery()
//-----------------------------------------------------------------------------
{
	m_queryHandle = ptCreateFrustumPointsQuery();

	// Final point color will be computed - no need to worry about intensity and so on 
	ptSetQueryRGBMode( m_queryHandle, PT_QUERY_RGB_MODE_SHADER );
}
//-----------------------------------------------------------------------------
void QueryRender::setPointLimit( int maxPnts )
//-----------------------------------------------------------------------------
{
	m_maxPoints = maxPnts;
}
//-----------------------------------------------------------------------------
void QueryRender::drawBackground()
//-----------------------------------------------------------------------------
{
	glClearColor( .0f, .0f, .0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}
//-----------------------------------------------------------------------------
bool QueryRender::drawPointClouds( bool dynamic, bool clearFrame )
//-----------------------------------------------------------------------------
{
	bool swap = false;

	prepareRender(clearFrame, dynamic);

	PThandle diagnostic = ptGetChannelByName(L"diagnostic");

	{
		ptResetQuery( m_queryHandle );

		pt::PerformanceTimer timer;
		timer.start();

		while (m_queryBuffer.executeQuery( m_queryHandle, diagnostic ))
		{
			glPointSize(dynamic ? 2.0f : 1.0f);

			glEnable( GL_COLOR_MATERIAL );
			glDisable( GL_LIGHTING );

			glEnableClientState( GL_VERTEX_ARRAY );
			glEnableClientState( GL_COLOR_ARRAY );

			glVertexPointer( 3, GL_FLOAT, 0, m_queryBuffer.getPointsBuffer() );

			if (diagnostic)
				glColorPointer(3, GL_UNSIGNED_BYTE, 0, m_queryBuffer.getChannelBuffer() );
			else
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
void QueryRender::prepareRender( bool clearFrame, bool dynamic )
//-----------------------------------------------------------------------------
{
	ptSetViewport( 0 );	// important to do this in OpenGL context
	ptReadViewFromGL();

	if ( ptNumScenes() )
	{
		if (!m_queryHandle) createFrustumQuery();

		m_queryBuffer.initialize( true, false );	// harmless to recall 

		ptStartDrawFrameMetrics();
	}
	if ( clearFrame )
	{
		ptResetQuery( m_queryHandle );
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}

	/* for dynamic viewing reduce density */ 
	if (m_maxPoints && !dynamic)
		ptSetQueryDensity( m_queryHandle, PT_QUERY_DENSITY_LIMIT, dynamic ? 1e5 : m_maxPoints );	
	else 
		ptSetQueryDensity( m_queryHandle, PT_QUERY_DENSITY_VIEW, dynamic ? 0.05f : 1.0f);
}
