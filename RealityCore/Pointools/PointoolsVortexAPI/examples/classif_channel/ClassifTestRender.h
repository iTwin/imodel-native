/******************************************************************************

Pointools Vortex API Examples

ChannelTestRenderer.h

Provides query based rendering for channels example 
demonstrates channel querying

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_CLASSIFICATION_CHANNEL_RENDERER_H_
#define POINTOOLS_CLASSIFICATION_CHANNEL_RENDERER_H_

#include "VortexExampleApp.h"
#include "QueryRender.h"

class ClassifTestRenderer : public QueryRender
{
public:
	ClassifTestRenderer(int buffersize=1e6);

	bool			drawPointClouds( bool dynamic, bool clearFrame );
	void			setClassifUserChannel( PThandle uc );

private:
	void			modifyPointsWithChannelValues();

	void			initializeColMap();
	const PTubyte	*mapClassificationToColor();

	PTubyte			*m_classificationRGB;
	PTubyte			m_classColMap[256*3];
	bool			m_mapInit;

	PThandle		m_channel;

};

#endif

