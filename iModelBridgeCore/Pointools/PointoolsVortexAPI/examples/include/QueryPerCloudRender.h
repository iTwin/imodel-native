/******************************************************************************

Pointools Vortex API Examples

QueryRender.h

Provides query based rendering - demonstrates query capability and rendering
values from user channels

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_QUERY_CLOUD_RENDERER_H_
#define POINTOOLS_EXAMPLE_QUERY_CLOUD_RENDERER_H_

#include "VortexExampleApp.h"
#include "QueryBuffer.h"

#include <map>

class QueryPerCloudRender : public Renderer
{
public:
	QueryPerCloudRender(int buffersize=1e6);

	void			drawBackground();
	bool			drawPointClouds( bool dynamic, bool clearFrame );
	void			setPointLimit(int maxPnts );

protected:
	void			prepareRender( bool clearFrame, bool dynamic );
	void			createFrustumQueries();

	void			resetQueries();
	void			setQueryDensities( PTenum densityType, PTfloat density );

	QueryBufferf							m_queryBuffer;
	
	typedef std::map<PThandle, PThandle>	QueryByCloud;
	QueryByCloud							m_queryHandles;

	int										m_maxPoints;
};

#endif

