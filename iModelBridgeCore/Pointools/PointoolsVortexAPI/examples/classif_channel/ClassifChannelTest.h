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
#include "ClassifTestRender.h"

class ClassifChannelTest : public Tool
{
public:
	
	enum
	{
		CmdLoadChannelsFile	= 1001,
		CmdSaveChannelsFile	= 1002,
		CmdDestroyChannel	= 1003,
		CmdSetClass1 = 1004,
		CmdSetClass2 = 1005,
		CmdSetClass3 = 1006,
		CmdSetClass4 = 1007,
		CmdSetClass5 = 1008,
		CmdSetClass6 = 1009,
		CmdSetClass7 = 1010,
		CmdSetClass8 = 1011,
		CmdSetClass9 = 1012,
		CmdSetClass10 = 1013,
                CmdCloneHandle = 1014,
	};

	ClassifChannelTest();

	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );
	
private:
	void			setSelectedPtsClass( int classID );
	void			loadChannelsFile();
	void			saveChannelsFile();
	void			destroyChannels();
	void			reset();

	// channel handles
	PThandle			m_classifChannel;

	int					m_isOoc;
	QueryBufferf		m_queryBuffer;
	
	ClassifTestRenderer	m_renderer;
};

#endif

