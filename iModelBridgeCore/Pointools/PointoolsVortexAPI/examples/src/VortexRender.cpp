/******************************************************************************

Pointools Vortex API Examples

VortexRender.cpp

Simple pass through to Vortex OpenGL based rendering

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

//----------------------------------------------------------------------------

*******************************************************************************/
#include "VortexRender.h"


// Pip Test

bool visTestInitialized = false;

void VortexRender::visTest(void)
{
	const unsigned int	numPoints = 128*1024;


	if(visTestInitialized == false)
	{
		queryVis = ptCreateVisPointsQuery();

		ptSetQueryDensity(queryVis,	1, 1.0);

		bufferGeom		= new double[numPoints * 3];
		bufferRGB		= new PTubyte[numPoints * 3];
		bufferIntensity	= new PTshort[numPoints];

		visTestInitialized = true;
	}

	unsigned int	n;
	unsigned long	totalPoints = 0;

	do
	{
		n = ptGetDetailedQueryPointsd(queryVis, numPoints, bufferGeom, bufferRGB, bufferIntensity, NULL, NULL, NULL, 0, NULL, NULL);

		totalPoints += n;

	} while(n > 0);

//	ptResetQuery(queryVis);

}


void VortexRender::drawBackground()
{
	glClearColor( .0f, .0f, .0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}
bool VortexRender::drawPointClouds( bool dynamic, bool clearFrame )
{
	ptSetViewport( 0 );	// important to do this in OpenGL context
	ptReadViewFromGL();

	if ( ptNumScenes() )
	{
		ptDrawGL( dynamic );

//visTest();

		return true;
	}

	return false;
}


