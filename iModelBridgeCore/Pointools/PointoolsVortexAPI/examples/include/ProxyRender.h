/******************************************************************************

Pointools Vortex API Examples

ProxyRender.h

Renders results from ptGetSceneProxyPoints

(c) Copyright 2008-12 Pointools Ltd

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_PROXY_RENDERER_H_
#define POINTOOLS_EXAMPLE_PROXY_RENDERER_H_

#include "VortexExampleApp.h"

class ProxyRender : public Renderer
{
public:

	double			*bufferGeom;
	PTubyte			*bufferRGB;

public:
	void			drawBackground();
	bool			drawPointClouds(bool dynamic, bool clearFrame );

	void			visTest(void);
};

#endif

