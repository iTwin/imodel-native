#pragma once

#include <ptui/ptWindow.h>
#include <ptui/ptPanel.h>
#include <ptui/ptuiprefs.h>
#include <ptui/ptalertbox.h>
#include <ptui/ptgrid.h>
#include <ptui/ptui_parser.h>
#include <fltk/input.h>
#include <fltk/run.h>
#include <fltk/ask.h>
#include <fltk/x.h>

#include "graph.h"

#include <map>
#include <vector>

//
// Interactor class
//
class MonitorInteractor : public ptui::Interactor
{
public:
	ptapp::CommandPipe::Result call(const char*cmd);
	void tooltip(const char*) {};
	void resetTooltip() {};
};


//
// UI class
//
class UI
{
	friend class App;

	UI();
	~UI();
	
	bool build();
	//const char* getOpenFilename(const char *desc, const char*ext);
	
public:
	void update() { mainWin()->updatePanels(); }

	ptui::Win*  mainWin()				{ return m_mainWin; }
	ptui::Grid* queriesGrid()			{ return m_qGrid; }
	ptui::Grid* loadGrid()				{ return m_lGrid; }
	ptui::Grid* visGrid()				{ return m_vGrid; }

	ptui::OutputGraph *queryGraph()		{ return m_qGraph; }
	ptui::OutputGraph *visGraph()		{ return m_vGraph; }
	ptui::OutputGraph *loadGraph()		{ return m_lGraph; }

	int activePanelIndex() const { return m_mainWin->panel(0)->visible() ? 0 : 1; }

private:
	ptui::Win	*m_mainWin;
	ptui::Grid	*m_qGrid;
	ptui::Grid  *m_lGrid;
	ptui::Grid  *m_vGrid;

	pt::ParameterMap m_pmap;
	ptui::ImageArray m_images;
	ptui::OutputGraph *m_qGraph;
	ptui::OutputGraph *m_vGraph;
	ptui::OutputGraph *m_lGraph;

	typedef std::map<pt::String, ptui::Win*> WinMap;
	WinMap m_windows;
	
	int buildWindowsFromScript( const char* filepath );
};

//
// Application Object
//
class App
{
public:
	App();
	~App();

	UI &ui() { return m_ui; }
	static App* instance();
	
	void run();
	
	bool initialiseApp();

	const char* applicationFolderA() const { return m_appFolderA; }
	const wchar_t*applicationFolderW() const { return m_appFolderW; }

	pt::ParameterMap &pmap() { return m_ui.m_pmap; }
private:
	UI		m_ui;
	wchar_t m_appFolderW[MAX_PATH];
	char	m_appFolderA[MAX_PATH];
};




