/******************************************************************************

Pointools Vortex API Examples

TuningTool.h

Demonstrates Vortex tuning options

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_TUNING_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_TUNING_TOOL_H_

#include "VortexExampleApp.h"
#include "..\include\VortexRender.h"
#include "..\include\QueryRender.h"
#include "..\include\QueryPerCloudRender.h"
//#include "..\include\ProxyRender.h"

class TuningTool : public Tool
{
public:
	enum
	{
		CmdUpdateTuningParams	= 400,
		CmdUpdateRenderer		= 401,
		CmdUpdateDynamic		= 402
	};

	TuningTool();

	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );
private:

	int m_cacheSize;
	int m_loadBias;
	int m_queryLimit;
	int m_pointsCap;
	int m_renderer;
	int m_dynamic;

	VortexRender		m_vortexRender;
	QueryRender			m_queryRender;
	QueryPerCloudRender m_queryPerCloudRender;
//	ProxyRender			m_proxyRender;
};

#endif

