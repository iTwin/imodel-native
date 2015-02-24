/******************************************************************************

Pointools Vortex API Examples

ClassificationRenderer.h

Provides query based rendering for rendering classifications

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_CLASSIFICATION_RENDERER_H_
#define POINTOOLS_EXAMPLE_CLASSIFICATION_RENDERER_H_

#include "VortexExampleApp.h"
#include "QueryRender.h"

class ClassificationRenderer : public QueryRender
{
public:
	ClassificationRenderer(int buffersize=1e6);

	bool			drawPointClouds( bool dynamic, bool clearFrame );
	
private:
	void			initializeColMap();
	const PTubyte	*mapClassificationToColor();

	PTubyte			*m_classificationRGB;
	PTubyte			m_classColMap[256*3];
	bool			m_mapInit;
};

#endif

