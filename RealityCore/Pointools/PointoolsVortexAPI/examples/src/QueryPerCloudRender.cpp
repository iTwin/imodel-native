/******************************************************************************

Pointools Vortex API Examples

QueryPerCloudRender.cpp

Provides query based rendering - demonstrates query capability and rendering
values from user channels

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

//----------------------------------------------------------------------------

*******************************************************************************/
#include "QueryPerCloudRender.h"

//-----------------------------------------------------------------------------
QueryPerCloudRender::QueryPerCloudRender(int buffersize) : m_queryBuffer(buffersize)
//-----------------------------------------------------------------------------
{
	pt::PerformanceTimer::initialize();
	m_maxPoints = 0;
}
//-----------------------------------------------------------------------------
void QueryPerCloudRender::createFrustumQueries(void)
//-----------------------------------------------------------------------------
{
	PThandle handles[256]; // not safe, > 256 point clouds is bad
	int numClouds = ptGetSceneHandles( handles );

	for (int i=0; i<numClouds; i++)
	{
		if (m_queryHandles.find(handles[i])== m_queryHandles.end())
		{
			PThandle query = ptCreateFrustumPointsQuery();

			ptSetQueryScope( query, handles[i] );	// limit query to point cloud
			ptSetQueryRGBMode( query, PT_QUERY_RGB_MODE_SHADER );

			m_queryHandles.insert( QueryByCloud::value_type( handles[i], query ) );
		}
	}
}
//-----------------------------------------------------------------------------
void QueryPerCloudRender::setPointLimit( int maxPnts )
//-----------------------------------------------------------------------------
{
	m_maxPoints = maxPnts;
}
//-----------------------------------------------------------------------------
void QueryPerCloudRender::drawBackground()
//-----------------------------------------------------------------------------
{
	glClearColor( .0f, .0f, .0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}
//-----------------------------------------------------------------------------
bool QueryPerCloudRender::drawPointClouds( bool dynamic, bool clearFrame )
//-----------------------------------------------------------------------------
{
	bool swap = false;

	prepareRender(clearFrame, dynamic);

	PThandle diagnostic = ptGetChannelByName(L"diagnostic");

	{
		QueryByCloud::iterator i = m_queryHandles.begin();

		while (i!=m_queryHandles.end())
		{
			ptResetQuery( i->second );

			while (m_queryBuffer.executeQuery( i->second, diagnostic ))
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
			}
			++i;
		}
		ptEndDrawFrameMetrics();
		return !(m_queryBuffer.isBufferFull());	// is view complete? if buffer is full it means there is another iteration to come
	}
	return true;
}
//-----------------------------------------------------------------------------
void QueryPerCloudRender::prepareRender( bool clearFrame, bool dynamic )
//-----------------------------------------------------------------------------
{
	ptSetViewport( 0 );	// important to do this in OpenGL context
	ptReadViewFromGL();

	if ( ptNumScenes() )
	{
		createFrustumQueries();

		m_queryBuffer.initialize( true, false );	// harmless to recall 

		ptStartDrawFrameMetrics();
	}
	if ( clearFrame )
	{
		resetQueries();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}

	/* for dynamic viewing reduce density */ 
	if (m_maxPoints && !dynamic)
		setQueryDensities( PT_QUERY_DENSITY_LIMIT, dynamic ? 1e5 : m_maxPoints );
	else 
		setQueryDensities( PT_QUERY_DENSITY_VIEW, dynamic ? 0.05f : 1.0f );
}
//-----------------------------------------------------------------------------
void QueryPerCloudRender::resetQueries()
//-----------------------------------------------------------------------------
{
	QueryByCloud::iterator i = m_queryHandles.begin();

	while (i!=m_queryHandles.end())
	{
		ptResetQuery( i->second );
		++i;
	}
}
//-----------------------------------------------------------------------------
void QueryPerCloudRender::setQueryDensities( PTenum densityType, PTfloat density )
//-----------------------------------------------------------------------------
{
	QueryByCloud::iterator i = m_queryHandles.begin();

	while (i!=m_queryHandles.end())
	{
		ptSetQueryDensity( i->second, PT_QUERY_DENSITY_LIMIT, density );	
		++i;
	}
}
