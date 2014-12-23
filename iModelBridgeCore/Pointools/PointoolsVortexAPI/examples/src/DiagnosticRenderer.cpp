/******************************************************************************

Pointools Vortex API Examples

DiagnosticRenderer.cpp

Provides query based rendering for rendering classifications

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/

#include "DiagnosticRenderer.h"

//-----------------------------------------------------------------------------
DiagnosticRenderer::DiagnosticRenderer(int buffersize) 
: QueryRender(buffersize)
//-----------------------------------------------------------------------------
{
	m_mapInit = false;
	m_channel = 0;
	m_diagnosticRGB = new PTubyte[buffersize*3];	// could throw
}

//-----------------------------------------------------------------------------
bool DiagnosticRenderer::drawPointClouds(bool dynamic, bool clearFrame)
//-----------------------------------------------------------------------------
{
	if (!m_queryBuffer.initialize(false, false, true)) return false;	// allocation failure

	prepareRender(clearFrame, dynamic);

	{
		ptResetQuery( m_queryHandle );

		pt::PerformanceTimer timer;
		timer.start();

		while (m_queryBuffer.executeQuery( m_queryHandle, m_channel ))
		{
			// do the actual draw using a GL vertex array
			glPointSize( dynamic ? 2.0f : 1.0f);

			glEnable( GL_COLOR_MATERIAL );
			glDisable( GL_LIGHTING );

			glEnableClientState( GL_VERTEX_ARRAY );
			glEnableClientState( GL_COLOR_ARRAY );

			// point positions
			glVertexPointer( 3, GL_FLOAT, 0, m_queryBuffer.getPointsBuffer() );

			// note in OpenGL texture coords cannot be single bytes, so we can't use texturing to
			// map classification values to colors without conversion to short or float.
			// The best way to render from bytes is in a shader, but to keep
			// this example simple we 'manually' re-colour points buffer to render classification
			// point color
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, mapDiagnosticToColor() );

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
void DiagnosticRenderer::initializeColMap()
//-----------------------------------------------------------------------------
{
	// Random Colors
	int i;
	for ( i=0; i<sizeof(m_colMap); i+=3)
	{
		m_colMap[i] = 64 + (192 * rand()) / RAND_MAX;
		m_colMap[i+1] = 64 + (192 * rand()) / RAND_MAX;;
		m_colMap[i+2] = 64 + (192 * rand()) / RAND_MAX;;
	}
}
//-----------------------------------------------------------------------------
const PTubyte	* DiagnosticRenderer::mapDiagnosticToColor()
//-----------------------------------------------------------------------------
{
	if (!m_mapInit)
	{
		initializeColMap();
	}
	m_mapInit = true;

	const PTubyte *diagnostic = reinterpret_cast<const PTubyte *>(m_queryBuffer.getChannelBuffer());
	if (!diagnostic) return 0;	// should only happen on allocation failure, will cause a crash - this is not 'production' code!

	int numPoints = m_queryBuffer.numPntsInQueryIteration();
	int i=0;

	for (i=0;i<numPoints;i++)
	{
		m_diagnosticRGB[i*3] = m_colMap[ diagnostic[i]*3 ];
		m_diagnosticRGB[i*3+1] = m_colMap[ diagnostic[i]*3+1 ];
		m_diagnosticRGB[i*3+2] = m_colMap[ diagnostic[i]*3+2 ];
	}
	return m_diagnosticRGB;
}

void DiagnosticRenderer::setDiagnosticChannel( PThandle channel )
{
	m_channel = channel;
}
