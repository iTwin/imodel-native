/******************************************************************************

Pointools Vortex API Examples

TuningTool.cpp

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "TuningTool.h"

//-----------------------------------------------------------------------------
void TuningTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch (cmdId)
	{
	case CmdUpdateRenderer:
		
		switch (m_renderer)
		{
		case 0: VortexExampleApp::instance()->setRenderer( &m_vortexRender );
			break;
		case 1:	VortexExampleApp::instance()->setRenderer( &m_queryRender );
			break;
		case 2: VortexExampleApp::instance()->setRenderer( &m_queryPerCloudRender ); 
			break;
		}
		Tool::viewRedraw();
		break;

	case CmdUpdateTuningParams:
		if (m_cacheSize >= 0) 
		{
			ptSetCacheSizeMb( m_cacheSize );
		}
		else ptAutoCacheSize();

		ptSetLoadingPriorityBias( m_loadBias );
		ptSetViewportPointsBudget( m_pointsCap );

		m_queryRender.setPointLimit( m_queryLimit );
		m_queryPerCloudRender.setPointLimit( m_queryLimit );

		break;

	case CmdUpdateDynamic:
		VortexExampleApp::instance()->getView().enableDynamic = (m_dynamic ? true : false);
		Tool::viewRedraw();
		break;
	}
}

//-----------------------------------------------------------------------------
void TuningTool::buildUserInterface( GLUI_Node *parent )
//-----------------------------------------------------------------------------
{
	/* tuning */ 
	GLUI_Rollout* rolloutTuning = new GLUI_Rollout(parent, "Tuning", true );
	GLUI_StaticText * spacer = new GLUI_StaticText( rolloutTuning, "" );
	spacer->set_w( PANEL_WIDTH );

	// GLUI_Listbox causes a crash with the current x64 build that is linking to freeglut
#ifndef _M_X64
	GLUI_Listbox * list = new GLUI_Listbox( rolloutTuning, "Cache Size ", &m_cacheSize, CmdUpdateTuningParams ,&Tool::dispatchCmd);
	list->add_item( -1, "Auto size");
	list->add_item( 0, "0Mb");
	list->add_item( 100, "100Mb");
	list->add_item( 250, "250Mb");
	list->add_item( 500, "500Mb");
	list->add_item( 1024, "1Gb");
	list->add_item( 1536, "1.5Gb");
	list->add_item( 2048, "2Gb");
	list->add_item( 3072, "3Gb");

	list = new GLUI_Listbox( rolloutTuning, "Load Bias ", &m_loadBias,  CmdUpdateTuningParams ,&Tool::dispatchCmd);
	list->add_item( PT_LOADING_BIAS_SCREEN, "Screen (Default)");
	list->add_item( PT_LOADING_BIAS_NEAR, "Front");
	list->add_item( PT_LOADING_BIAS_FAR, "Back");
	list->add_item( PT_LOADING_BIAS_POINT, "Point");

	new GLUI_Button( rolloutTuning, "Set Bias Point",  CmdUpdateTuningParams ,&Tool::dispatchCmd);

	list = new GLUI_Listbox( rolloutTuning, "Query Render Limit ", &m_queryLimit,  CmdUpdateTuningParams ,&Tool::dispatchCmd);
	list->add_item( 0, "No Limit");
	list->add_item( 5e5, "500,000 pnts");
	list->add_item( 1e6, "1m pnts");
	list->add_item( 1.5e6, "1.5m pnts");
	list->add_item( 2.5e6, "2.5m pnts");
	list->add_item( 5e6, "5m pnts");
	list->add_item( 1e7, "10m pnts");
#endif // _M_X64

	spacer = new GLUI_StaticText( rolloutTuning, "" );
	spacer = new GLUI_StaticText( rolloutTuning, "Renderer" );

	GLUI_RadioGroup *renderer = new GLUI_RadioGroup( rolloutTuning, &m_renderer, CmdUpdateRenderer ,&Tool::dispatchCmd);
	new GLUI_RadioButton( renderer, "Vortex");
	new GLUI_Column(renderer, false);
	new GLUI_RadioButton( renderer, "Query");
	new GLUI_Column(renderer, false);
	new GLUI_RadioButton( renderer, "Cloud Query");
	
	spacer = new GLUI_StaticText( rolloutTuning, "" );
	spacer->set_w( PANEL_WIDTH );
	GLUI_Checkbox* dynamic = new GLUI_Checkbox( rolloutTuning, "Dynamic", &m_dynamic, CmdUpdateDynamic, &Tool::dispatchCmd);	
	spacer = new GLUI_StaticText( rolloutTuning, "" );
	spacer->set_w( PANEL_WIDTH );

#ifndef _M_X64
	list = new GLUI_Listbox( rolloutTuning, "Points Cap ", &m_pointsCap,  CmdUpdateTuningParams ,&Tool::dispatchCmd);
	list->add_item( -1, "No Limit");
	list->add_item( 5e5, "500,000 pnts");
	list->add_item( 1e6, "1m pnts");
	list->add_item( 1.5e6, "1.5m pnts");
	list->add_item( 2.5e6, "2.5m pnts");
	list->add_item( 5e6, "5m pnts");
	list->add_item( 1e7, "10m pnts");
#endif // _M_X64
}

//-----------------------------------------------------------------------------
TuningTool::TuningTool() : Tool(CmdUpdateTuningParams, CmdUpdateDynamic)
//-----------------------------------------------------------------------------
{
	m_cacheSize=0;
	m_loadBias=0;
	m_queryLimit=0;
	m_renderer=0;
	m_pointsCap=0;
	m_dynamic = 1;

	VortexExampleApp::instance()->setRenderer( &m_vortexRender );
}
