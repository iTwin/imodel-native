/******************************************************************************

Pointools Vortex API Examples

VortexRender.h

Simple pass through to Vortex OpenGL based rendering

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_VORTEX_RENDERER_H_
#define POINTOOLS_EXAMPLE_VORTEX_RENDERER_H_

#include "VortexExampleApp.h"

class VortexRender : public Renderer
{
public:

	PThandle		queryVis; 
	double			*bufferGeom;
	PTubyte			*bufferRGB;
	PTshort			*bufferIntensity;

public:
	void			drawBackground();
	bool			drawPointClouds(bool dynamic, bool clearFrame );

	void			visTest(void);
};

#endif

