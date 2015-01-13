/******************************************************************************

Pointools Vortex API Examples

FileTool.h

Demonstrates file loading and closing

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#include "FileTool.h"


//-----------------------------------------------------------------------------
void FileTool::command( int cmdId )
//-----------------------------------------------------------------------------
{

	switch (cmdId)
	{
	case CmdFileOpen:

		ptBrowseAndOpenPOD();

		if (ptNumScenes())
			viewRedraw();

		VortexExampleApp::instance()->notifySceneUpdate();

		break;

	case CmdFileCloseAll:
		ptRemoveAll();
		viewRedraw();

		VortexExampleApp::instance()->notifySceneUpdate();

		break;
	}
}
//-----------------------------------------------------------------------------
void FileTool::onIdle()												
//-----------------------------------------------------------------------------
{
	View &view = VortexExampleApp::instance()->getView();

	double kb = ptKbLoaded(false);

	if (view.ownDisplay)
	{
		ptKbLoaded(true); // reset counter
		glutPostRedisplay();		
	}
	else if (kb > 4096)
	{
		view.timeSinceKbUpdate.end();
		kb /= 1024;

		static char mbs[128];
		static double total = 0;
		static double hi = 0;
		static double lo = 200;
		total += kb;

		kb /= 0.001 * view.timeSinceKbUpdate.millisecs();

		if (hi < kb) hi = kb;
		if (lo > kb) lo = kb;

		sprintf(mbs, "Load Stats:\nLast %0.1fMb/s\nHigh %0.1fMb/s\nLow %0.1fMb/s\nTotal %0.1fMb", kb, hi, lo, total);
		m_txtMbSec->set_text(mbs);

		ptKbLoaded(true); // reset counter
		glutPostRedisplay();
	}
}
//-----------------------------------------------------------------------------
void FileTool::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	GLUI_Rollout *rolloutFile = new GLUI_Rollout( parent, "File", true );

	rolloutFile->set_w( PANEL_WIDTH );

	GLUI_StaticText *spacer = new GLUI_StaticText( rolloutFile, "" );
	spacer->set_w( PANEL_WIDTH );

	GLUI_Panel *pan = new GLUI_Panel( rolloutFile, " ", GLUI_PANEL_NONE);

	new GLUI_Button( pan, "Open", CmdFileOpen, &Tool::dispatchCmd  );
	
	GLUI_Column *col = new GLUI_Column( pan, false );
	
	new GLUI_Button( pan, "Close All", CmdFileCloseAll, &Tool::dispatchCmd  );

	// Scene Information
	new GLUI_StaticText( rolloutFile, "" );
	m_infoString = "Scene Information:\n-------------------";
	GLUI_TextBox * info = new GLUI_TextBox( rolloutFile, m_infoString, false );
	info->set_w( PANEL_WIDTH );
	info->set_h( 80 );
	info->align();

	// Stats
	new GLUI_StaticText( rolloutFile, "" );
	m_loadString = "Load stats";
	
	m_txtMbSec = new GLUI_TextBox( rolloutFile, m_loadString );
	m_txtMbSec->set_w( PANEL_WIDTH );
	m_txtMbSec->set_h( 80 );

	new GLUI_StaticText( rolloutFile, "" );
}