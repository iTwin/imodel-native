/******************************************************************************

Pointools Vortex API Examples

ChannelTestRenderer.h

Provides query based rendering for channels example 
demonstrates channel querying

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_CHANNEL_RENDERER_H_
#define POINTOOLS_EXAMPLE_CHANNEL_RENDERER_H_

#include "VortexExampleApp.h"
#include "QueryRender.h"

class ChannelTestRenderer : public QueryRender
{
public:
	ChannelTestRenderer(int buffersize=1e6);

	bool			drawPointClouds( bool dynamic, bool clearFrame );
	void			setChannels( PThandle offset, PThandle col );
	void			setMixer(float mixer);

private:
	void			modifyPointsWithChannelValues();
	PThandle		m_channels[2];
	float			m_mixer;
};

#endif

