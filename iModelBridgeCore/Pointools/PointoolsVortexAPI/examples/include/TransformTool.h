/******************************************************************************

Pointools Vortex API Examples

CloudVisibilityTool.h

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_CLOUD_VISIBILITY_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_CLOUD_TRANSFORM_TOOL_H_

#include "VortexExampleApp.h"

/** Simple tool that enables the transformation of the first scene loaded
 */
class TransformTool : public Tool
{
public:
	enum
	{
		CmdMoveLeft		= 8500,
		CmdMoveRight	= 8501,
		CmdMoveForward	= 8502,
		CmdMoveBack		= 8503,
		CmdMoveUp		= 8504,
		CmdMoveDown		= 8505,
		CmdMoveReset	= 8506
	};

	TransformTool();

	void buildUserInterface(GLUI_Node *parent);
	void command( int cmdId );
	
private:

};

#endif // POINTOOLS_EXAMPLE_APP_CLOUD_VISIBILITY_TOOL_H_

