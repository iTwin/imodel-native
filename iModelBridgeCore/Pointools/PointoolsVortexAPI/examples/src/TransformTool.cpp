/******************************************************************************

Pointools Vortex API Examples

TransformTOol.cpp

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "TransformTool.h"

//-----------------------------------------------------------------------------
TransformTool::TransformTool() :
Tool(CmdMoveLeft, CmdMoveReset)
//-----------------------------------------------------------------------------
{
 
}
//-----------------------------------------------------------------------------
void TransformTool::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	GLUI_Rollout *rolloutTransform = new GLUI_Rollout( parent, "Transform", true );

	rolloutTransform->set_w( PANEL_WIDTH );
	
	// Controls - Left Right
	GLUI_Button * l = new GLUI_Button( rolloutTransform, " " );
	l->set_w(8);
	GLUI_Button *b = new GLUI_Button( rolloutTransform, "L", CmdMoveLeft, &Tool::dispatchCmd );
	b->set_w(8);

	l = new GLUI_Button( rolloutTransform, " " );
	l->set_w(8);

	new GLUI_Column( rolloutTransform, false );
	
	// Forward / Back
	b = new GLUI_Button( rolloutTransform, "F", CmdMoveForward, &Tool::dispatchCmd );
	b->set_w(8);

	b = new GLUI_Button( rolloutTransform, "+", CmdMoveReset, &Tool::dispatchCmd );
	b->set_w(8);

	b = new GLUI_Button( rolloutTransform, "B", CmdMoveBack, &Tool::dispatchCmd );
	b->set_w(8);

	new GLUI_Column( rolloutTransform, false );

	// Right
	l = new GLUI_Button( rolloutTransform, " " );
	l->set_w(8);

	b = new GLUI_Button( rolloutTransform, "R", CmdMoveRight, &Tool::dispatchCmd );
	b->set_w(8);

	l = new GLUI_Button( rolloutTransform, " " );
	l->set_w(8);

	// Up Down
	new GLUI_Column( rolloutTransform, false );
	new GLUI_Column( rolloutTransform, false );
	l = new GLUI_Button( rolloutTransform, "U", CmdMoveUp, &Tool::dispatchCmd );
	l->set_w(8);

	b = new GLUI_Button( rolloutTransform, " " );
	b->set_w(8);

	l = new GLUI_Button( rolloutTransform, "D", CmdMoveDown, &Tool::dispatchCmd );
	l->set_w(8);
}
//-----------------------------------------------------------------------------
void TransformTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	PThandle handle = 0;
	
	PTdouble step = 1.0;

	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *handles = new PThandle[numScenes];
		ptGetSceneHandles( handles );

		handle = handles[0]; 
		delete [] handles;
	}
	if (handle)
	{
		PTdouble mat[16];

		memset(mat, 0, sizeof(mat));
		
		//read the current matrix
		ptGetSceneTransform( handle, mat, false);
		
		switch (cmdId)
		{
		case CmdMoveLeft:		mat[4*3+0] -= step; break;
		case CmdMoveRight:		mat[4*3+0] += step; break;
		case CmdMoveForward:	mat[4*3+1] += step; break;
		case CmdMoveBack:		mat[4*3+1] -= step; break;
		case CmdMoveUp:			mat[4*3+2] += step; break;
		case CmdMoveDown:		mat[4*3+2] -= step; break;
		case CmdMoveReset:		
			// make identity
			memset(mat, 0, sizeof(mat));
			mat[0] = mat[4*1 + 1] = mat[4*2 + 2] = mat[4*3 + 3] = 1.0;
			break;
		}
		ptSetSceneTransform( handle, mat, false);

		VortexExampleApp::instance()->updateBoundingBox();
		Tool::viewRedraw();
	}
}