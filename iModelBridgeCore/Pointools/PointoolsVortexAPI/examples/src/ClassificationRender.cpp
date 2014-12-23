/******************************************************************************

Pointools Vortex API Examples

ClassificationRenderer.cpp

Provides query based rendering for rendering classifications

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/

#include "ClassificationRender.h"

//-----------------------------------------------------------------------------
ClassificationRenderer::ClassificationRenderer(int buffersize) 
: QueryRender(buffersize)
//-----------------------------------------------------------------------------
{
	m_mapInit = false;
	m_classificationRGB = new PTubyte[buffersize*3];	// could throw
}

//-----------------------------------------------------------------------------
bool ClassificationRenderer::drawPointClouds(bool dynamic, bool clearFrame)
//-----------------------------------------------------------------------------
{
	if (!m_queryBuffer.initialize(false, false, true)) return false;	// allocation failure

	prepareRender(clearFrame, dynamic);

	{
		ptResetQuery( m_queryHandle );

		pt::PerformanceTimer timer;
		timer.start();

		while (m_queryBuffer.executeQuery( m_queryHandle ))
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
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, mapClassificationToColor() );
			
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
void ClassificationRenderer::initializeColMap()
//-----------------------------------------------------------------------------
{
	// Random Colors
	int i;
	for ( i=0; i<sizeof(m_classColMap); i+=3)
	{
		m_classColMap[i] = 64 + (192 * rand()) / RAND_MAX;
		m_classColMap[i+1] = 64 + (192 * rand()) / RAND_MAX;;
		m_classColMap[i+2] = 64 + (192 * rand()) / RAND_MAX;;
	}
	
	// LAS default colors, first 10 class values
	static PTubyte LAS_Defaults [][3] = 
	{
		{255, 255, 0},			//    eCreated = 0,
		{255, 255, 255},		//    eUnclassified,
		{255, 0, 0},			//    eGround,
		{0, 200, 50},			//    eLowVegetation,
		{0, 200, 100},			//    eMediumVegetation,
		{0, 200, 200},			//    eHighVegetation,
		{150, 150, 150},		//    eBuilding,
		{0,	50, 100},			//    eLowPoint,
		{255, 255, 255},		//    eModelKeyPoint,
		{0, 0, 255},			//    eWater = 9,
	};
	for (i=0; i<sizeof(LAS_Defaults); i++)
	{
		m_classColMap[i]=((PTubyte*)LAS_Defaults)[i];
	}
}
//-----------------------------------------------------------------------------
const PTubyte	* ClassificationRenderer::mapClassificationToColor()
//-----------------------------------------------------------------------------
{
	if (!m_mapInit)
	{
		initializeColMap();
	}
	m_mapInit = true;

	const PTubyte *classification = m_queryBuffer.getClassificationBuffer();
	if (!classification) return 0;	// should only happen on allocation failure, will cause a crash - this is not 'production' code!

	int numPoints = m_queryBuffer.numPntsInQueryIteration();
	int i=0;

	for (i=0;i<numPoints;i++)
	{
		m_classificationRGB[i*3] = m_classColMap[ classification[i]*3 ];
		m_classificationRGB[i*3+1] = m_classColMap[ classification[i]*3+1 ];
		m_classificationRGB[i*3+2] = m_classColMap[ classification[i]*3+2 ];
	}
	return m_classificationRGB;
}
