/******************************************************************************

Pointools Vortex API Examples

BitmapTool.h

Demonstrates creating a Bitmap Viewport and saving a screenshot

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_BITMAP_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_BITMAP_TOOL_H_

#include "VortexExampleApp.h"

class BitmapTool : public Tool
{
public:
	enum
	{
		CmdCreateBitmap = 800
	};

	BitmapTool();

	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );

private:
	void	createBitmap();
};

#endif

