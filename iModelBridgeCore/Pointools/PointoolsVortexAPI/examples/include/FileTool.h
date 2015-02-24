/******************************************************************************

Pointools Vortex API Examples

FileTool.h

Demonstrates file loading and closing

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_FILE_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_FILE_TOOL_H_

#include "VortexExampleApp.h"

class FileTool : public Tool
{
public:
	enum
	{
		CmdFileOpen = 100,
		CmdFileCloseAll = 101
	};

	FileTool() : Tool(CmdFileOpen, CmdFileCloseAll) {}
	
	void	onIdle();
	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );

private:
	std::string		m_infoString;
	std::string		m_loadString;
	GLUI_TextBox*	m_txtMbSec;
};

#endif

