/******************************************************************************

Pointools Vortex API Examples

ChannelsTest.h

Demonstrates basic client server operation

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_CHANNELS_TEST_H_
#define POINTOOLS_EXAMPLE_APP_CHANNELS_TEST_H_

#include "VortexExampleApp.h"
#include "QueryBuffer.h"
#include "ChannelTestRender.h"

class ChannelsTest : public Tool
{
public:
	
	enum
	{
		CmdDoChannelsTest				= 600,
		CmdLoadChannelsFile				= 601,
		CmdCopyChannelsFile 			= 602,
		CmdWriteReadChannelsFileBuffer	= 603,
		CmdSaveChannelsFile				= 604,
		CmdDestroyChannel				= 605,
		CmdUpdateMixer					= 606,
		CmdSavePODFile					= 607,
		CmdBoxSelect					= 608,
		CmdBoxRotate					= 609
	};

	ChannelsTest();

	void		buildUserInterface(GLUI_Node *parent);
	void		command( int cmdId );

private:
	
	void		doChannelsTestOnOrientedBox();
	void		doChannelsTestOnSelected();
	void		doChannelsTest(PThandle queryHandle);

	void		loadChannelsFile();
	void		copyPointChannels();
	void		writeReadPointChannelsBuffer();
	void		saveChannelsFile();
	void		destroyChannels();
	void		updateChannelsRenderer();
	void		savePOD();
	void		reset();

	// channel handles
	PThandle	m_colChannel;
	PThandle	m_offChannel;

	float		m_effectMixer;
	int			m_isOoc;
	int			m_isCopyOoc;

	QueryBufferf		m_queryBuffer;
	ChannelTestRenderer	m_renderer;

	// box select stuff
	bool		onMouseButtonDown( int button, int x, int y );
	bool		onMouseButtonUp( int button, int x, int y );
	bool		onMouseDrag( int x, int y, int startX, int startY );

	void		drawPostDisplay();

	int			m_mode;
	Mouse		m_mouse;

	PTfloat		m_boxLower[3];
	PTfloat		m_boxUpper[3];
	PTbool		m_boxValid;

	float		m_boxColor[3];
	float		m_boxPosition[3];
	float		m_boxRotation[3];

	int			copyCounter;
};

#endif

