/******************************************************************************

Pointools Vortex API Examples

QueryRender.h

Provides query based rendering - demonstrates query capability and rendering
values from user channels

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_QUERY_RENDERER_H_
#define POINTOOLS_EXAMPLE_QUERY_RENDERER_H_

#include "VortexExampleApp.h"
#include "QueryBuffer.h"

class QueryRender : public Renderer
{
public:
	QueryRender(int buffersize=1e6);

	void			drawBackground();
	bool			drawPointClouds( bool dynamic, bool clearFrame );
	void			setPointLimit(int maxPnts );

protected:
	void			prepareRender( bool clearFrame, bool dynamic );
	void			createFrustumQuery();

	QueryBufferf	m_queryBuffer;
	PThandle		m_queryHandle;
	int				m_maxPoints;
};

#endif

