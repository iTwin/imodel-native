/******************************************************************************

Pointools Vortex API Examples

CloudVisibilityTool.cpp

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "CloudVisibilityTool.h"
#include "FileTool.h"

//-----------------------------------------------------------------------------
CloudVisibilityTool::CloudVisibilityTool() :
Tool(FileTool::CmdFileOpen, CmdHideCloud)
//-----------------------------------------------------------------------------
{
	m_cloudTextBox = NULL;
}
//-----------------------------------------------------------------------------
void CloudVisibilityTool::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	GLUI_Rollout *rolloutFile = new GLUI_Rollout( parent, "Cloud Visibility", true );

	rolloutFile->set_w( PANEL_WIDTH );

	GLUI_StaticText *spacer = new GLUI_StaticText( rolloutFile, "" );
	spacer->set_w( PANEL_WIDTH );

	// Stats
	new GLUI_StaticText( rolloutFile, "" );
	m_cloudInfo = "Cloud Info";
	m_cloudTextBox = new GLUI_List( rolloutFile, m_cloudInfo );
	m_cloudTextBox->set_w( PANEL_WIDTH );
	m_cloudTextBox->set_h( 80 );
	new GLUI_StaticText( rolloutFile, "" );

	GLUI_Panel *pan = new GLUI_Panel( rolloutFile, " ", GLUI_PANEL_NONE);
	new GLUI_Button( pan, "Show", CmdShowCloud, &Tool::dispatchCmd  );
	
	GLUI_Column *col = new GLUI_Column( pan, false );
	new GLUI_Button( pan, "Hide", CmdHideCloud, &Tool::dispatchCmd  );	
}
//-----------------------------------------------------------------------------
void CloudVisibilityTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch (cmdId)
	{
		case CmdShowCloud:
		{	
			PThandle h = getSelectedCloudHandle();
			if (h != PT_NULL)
				ptShowScene(h, true);
			viewRedraw();
			break;
		}

		case CmdHideCloud:
		{	
			PThandle h = getSelectedCloudHandle();
			if (h != PT_NULL)
				ptShowScene(h, false);
			viewRedraw();
			break;
		}
		
		// Commands from FileTool
		case FileTool::CmdFileOpen:
			// a new cloud has been opened, add it to the list of clouds
			updateCloudList();
			break;

		case FileTool::CmdFileCloseAll:
			// remove all clouds from the list
			updateCloudList();
			break;
	}
}
//-----------------------------------------------------------------------------
PThandle CloudVisibilityTool::getSelectedCloudHandle()
//-----------------------------------------------------------------------------
{
	if (m_cloudTextBox)
	{
		unsigned int selected = m_cloudTextBox->get_current_item();
		if (selected < 0)
			return PT_NULL;

		if (selected < m_cloudList.size())
			return m_cloudList[selected];
	}

	return PT_NULL;
}

void CloudVisibilityTool::updateCloudList()
{
	if (!m_cloudTextBox)
		return;

	m_cloudList.clear();
	m_cloudTextBox->delete_all();

	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *sceneHandles = new PThandle[numScenes];
		if (ptGetSceneHandles(sceneHandles))
		{
			for (int i = 0; i < numScenes; i++)
			{
				m_cloudList.push_back(sceneHandles[i]);
				char handle[32] = {0};
				sprintf(handle, "%d", sceneHandles[i]);
				m_cloudTextBox->add_item(i, handle);
			}
		}
		delete [] sceneHandles;
	}	
}