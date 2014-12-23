#include "stdafx.h"
#include <commdlg.h>

#include "appFramework.h"
#include "graph.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>

#include <ptappdll/ptapp.h>
#include <pt/unicodeconversion.h>
#include <ptfs/filepath.h>
#include <ptlang/language.h>
#include <registry/registry_value.h>
#include <pt/utf.h>

using namespace ptui;

App::App()
{
}
App::~App()
{
}
UI::UI(){}
UI::~UI(){}
//
// singleton implmentation
//
App *App::instance()
{
	static App* app=0;
	if (!app)
	{
		app = new App;
	}
	return app;
}
//
// Run the app, show the window
//
void App::run()
{
	m_ui.mainWin()->exec();
	fltk::run();
}

//
// Init, build the UI
//
bool App::initialiseApp()
{
	::GetModuleFileNameW( NULL, m_appFolderW, MAX_PATH );
	::GetModuleFileNameA( NULL, m_appFolderA, MAX_PATH );
	::PathRemoveFileSpecW( m_appFolderW );
	::PathRemoveFileSpecA( m_appFolderA );
	BOOL success = SetCurrentDirectoryW( m_appFolderW );

	wchar_t config[MAX_PATH];
	ptui::Preferences::initialize();
	swprintf(config, L"%s\\config\\uiprefs.cfg", m_appFolderW );

	ptui::Preferences::initialize();
	ptui::Preferences::retrieve(config);

	char filepath[MAX_PATH];
	sprintf(filepath, "%s\\scripts\\VortexMonitor.script", applicationFolderA() );

	if (m_ui.buildWindowsFromScript( filepath ))
	{
		bool res = m_ui.build();
		if (res)
		{
			fltk::Window::first( m_ui.mainWin() );
			return true;
		}
	}
	return false;
}

//
// build the UI windows from ptui script
//
int UI::buildWindowsFromScript( const char* filepath )
{
	static MonitorInteractor interactor;

	ptui::Win::setInteractor( &interactor );
	pt::ParameterMap *dt = &m_pmap;

	if (_access(filepath, 4)==0)
	{
		std::ifstream ifs( filepath );
			
		char _error [256];
		ptuiParser parser(&ifs);

		try 
		{
			parser.file(dt);
		} 
		catch (ScanException &scex)
		{
			strcpy_s(_error, sizeof(_error), ((std::string)scex).c_str());
			
			fltk::alert("Scan Error in interface script file %s \n%s", filepath, _error);
			return false;
		}
		catch (ParseException &pex)
		{
			strcpy_s(_error, sizeof(_error), ((std::string)pex).c_str());

			fltk::alert("Parse Error in interface script file %s \n%s", filepath, _error);
			return false;
		}

		for (uint i=0; i<parser._windows.size(); i++)
		{
			m_windows.insert(WinMap::value_type(parser._identifiers[i].c_str(), parser._windows[i]));
		}

		WinMap::iterator w = m_windows.find("VortexMonitor");
		if (w != m_windows.end())
		{
			m_mainWin = w->second;
		}
		else return false;
	}
	else
	{
		char mess[512];
		sprintf(mess, "Can't access interface script %s", filepath);

		ptui::alertBox("Script Error", mess);		
		return false;
	}
	return true;
}

bool UI::build()
{	
	/* grid */ 
	m_qGrid = (ptui::Grid*)m_mainWin->panel(0)->getGrid("queries");
	m_qGrid->rowHeight(11);
	m_qGrid->textsize(8.0f);

	m_qGrid->header()->labelsize( 9.0f );
	m_qGrid->header()->insertItem(0, "Timestamp");
	m_qGrid->header()->insertItem(1, "Query ID");
	m_qGrid->header()->insertItem(2, "Type");
	m_qGrid->header()->insertItem(3, "num Points");
	m_qGrid->header()->insertItem(4, "Density Type");
	m_qGrid->header()->insertItem(5, "Density Val");
	m_qGrid->header()->insertItem(6, "Buffer Size");
	//m_qGrid->header()->insertItem(6, "Time");
	m_qGrid->columnWidth(0, 70);
	m_qGrid->columnWidth(1, 60);
	m_qGrid->columnWidth(2, 150);
	m_qGrid->columnWidth(3, 150);
	m_qGrid->columnWidth(4, 180);
	m_qGrid->columnWidth(5, 120);
	m_qGrid->columnWidth(6, 210);
	m_qGrid->columnWidth(7, 150);

	m_qGrid->selectMode( ptui::GridSelectRow );

	m_lGrid = (ptui::Grid*)m_mainWin->panel(1)->getGrid("loading");
	m_lGrid->rowHeight(11);
	m_lGrid->textsize(8.0f);
	m_lGrid->header()->labelsize( 9.0f );
	m_lGrid->header()->insertItem(0, "Timestamp");
	m_lGrid->header()->insertItem(1, "Shortfall (pts)");
	m_lGrid->header()->insertItem(2, "Loaded Since Draw (pts)");

	m_lGrid->selectMode( ptui::GridSelectRow);
	m_lGrid->columnWidth(0, 100);
	m_lGrid->columnWidth(1, 200); 
	m_lGrid->columnWidth(2, 300);

	m_vGrid = (ptui::Grid*)m_mainWin->panel(2)->getGrid("visibility");
	m_vGrid->rowHeight(11);
	m_vGrid->textsize(8.0f);

	m_vGrid->header()->labelsize( 9.0f );
	m_vGrid->header()->insertItem(0, "Timestamp");
	m_vGrid->header()->insertItem(1, "# voxels");
	m_vGrid->header()->insertItem(2, "# full pts");
	m_vGrid->header()->insertItem(3, "# lod pts");
	m_vGrid->header()->insertItem(4, "max lod");
	m_vGrid->header()->insertItem(5, "min lod");
	m_vGrid->header()->insertItem(6, "av lod");
	m_vGrid->header()->insertItem(7, "total proj area");
	m_vGrid->header()->insertItem(8, "overplot");
	m_vGrid->columnWidth(0, 70);
	m_vGrid->columnWidth(1, 100);
	m_vGrid->columnWidth(2, 100);
	m_vGrid->columnWidth(3, 100);
	m_vGrid->columnWidth(4, 100);
	m_vGrid->columnWidth(5, 100);
	m_vGrid->columnWidth(6, 100);
	m_vGrid->columnWidth(7, 100);

	m_vGrid->selectMode( ptui::GridSelectRow );
	//////////////////////////////////////////////
	/* menu */ 
	/* reposition for graphic at top */ 
//	m_mainWin->panelGroup()->position(0, 55);
	m_mainWin->relayout();
	int labelYpos = m_mainWin->h() - 50;

	m_mainWin->resize(100, 100, m_mainWin->w(), labelYpos);
	m_mainWin->updatePanels();
	m_mainWin->setFixedHeight(800);
	m_mainWin->relayout();

	// grpah
	m_qGraph = new ptui::OutputGraph( 5, 505, m_mainWin->w()-12, 250 );
	m_qGraph->setXRange( 0, 120000.0f );
	m_qGraph->setYRange( 1000000.0f, 5000000.0f );
	m_qGrid->set_visible();

	m_vGraph = new ptui::OutputGraph( 5, 505, m_mainWin->w()-12, 250 );
	m_vGraph->setXRange( 0, 120000.0f );
	m_vGraph->setYRange( 1000000.0f, 5000000.0f );
	m_vGraph->set_visible();

	m_lGraph = new ptui::OutputGraph( 5, 505, m_mainWin->w()-12, 250 );
	m_lGraph->setXRange( 0, 120000.0f );
	m_lGraph->setYRange( 1000000.0f, 5000000.0f );
	m_lGrid->set_visible();

	m_mainWin->panelGroup()->panel(0)->add( m_qGraph );
	m_mainWin->panelGroup()->panel(1)->add( m_lGraph );
	m_mainWin->panelGroup()->panel(2)->add( m_vGraph );

	/* copyright label */ 
	fltk::Widget *label = new fltk::Widget(10, labelYpos - 20, 400, 18, 
		"Pointools Vortex Monitor 1.2 Copyright (c) 2009-2012 Pointools Ltd. All rights reserved");

	label->labelsize( 9.0f );
	label->labelcolor( ptui::Preferences::color( ptui::Panel_Text_Color ) );
	label->color( m_mainWin->color() );
	label->align( fltk::ALIGN_INSIDE_TOPLEFT );
	label->box( fltk::NO_BOX );
	m_mainWin->add( label );

	return true;
}
