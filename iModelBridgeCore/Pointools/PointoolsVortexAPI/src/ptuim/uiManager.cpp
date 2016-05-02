/*--------------------------------------------------------------------------*/ 
/*	Pointools UIManager class definition									*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 16 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifdef WIN32
#pragma warning (disable : 4786)
#pragma warning (disable : 4251)
#pragma warning (disable : 4275)
#endif
#define COUTTRACER


#include <ptapp/uiparser/uiparser.h>
#include <ptui/ptui_parser.h>
#include <ptapp/application.h>
#include <ptapp/SettingsWin.h>

#include <ptcmdppe/commandpipe.h>
#include <ptuim/UIManager.h>
#include <ptui/ptWindow.h>

#include <fltk/ask.h>
#include <fltk/run.h>

#include <boost/bind.hpp>
#include <boost/filesystem\path.hpp>
#include <boost/filesystem\operations.hpp>
#include <boost/tokenizer.hpp>

#include <io.h>

#include <ptui/ptSplash.h>

#include <ptappdll/ptapp.h>
#include <ptl/project.h>

#include <ptapp/objectswin.h>
#include <ptapp/animatedobjects.h>
#include <ptvpm/viewports.h>
#include <ptui/ptalertbox.h>
#include <ptui/ptshortcuts.h>

#include <ptapp/viewportframe.h>
#include <ptui/ptviewport.h>
#include <pt/memrw.h>

#include <pt/trace.h>

using namespace ptapp;
using namespace ptui;
using namespace pt;
using namespace cppcc;

static const char* _lastWindow;
static void pointoolsFolder(char *d)
{
	LPITEMIDLIST pidl;
	if (SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl) == NOERROR)	
	{
		char p[MAX_PATH];
		SHGetPathFromIDList(pidl,d);
	}
}
namespace uiman
{
	static CommandPipe* cmdpipe()
	{
		static CommandPipe *cp = 0;
		if (!cp)
		{
			CommandPipe::RequestObj r;
			Application::app()->requestModule(&r);
			cp = r.module();
		}
		return cp;
	}
	UIManager *_instance = 0;
	ptl::BranchHandler *_bhandler = 0;
}

//cmdpipe()::CommandPipe	commandpipe;
//
// parser action
//
class _action : public uiparser::action 
{
public:
	_action(MainFrame*mf, std::vector<ImageArray*> *ia)	{	mframe = mf; _imagearrays = ia;}
	/*menubar*/ 
	void addMenubar(const char*name)		{	mframe->addMenubar();	};
	void addMenu(const char*caption, int img = -1) { mframe->lastMenubar()->addMenu(caption, img); };
	void addMenuItem(const char*caption, const char *cmd, int img = -1) 
	{ 
		const char*shortcut = 0;
		if (cmd)
		{
			shortcut = ptui::ShortcutsManager::instance()->getShortcutKey(cmd);
		}
		mframe->lastMenubar()->addItem(caption, cmd, shortcut, img); 
	};
	void addMenuDivider() { mframe->lastMenubar()->addDivider(); };
	void setMenubarImageArray(int i) { mframe->lastMenubar()->setImageArray((*_imagearrays)[i]); };

	/*image arrays*/ 
	void addImageArray() {	_imagearrays->push_back(new ImageArray()); }
	void addImage(const char *filename) 	{
		PTTRACEOUT << "adding image: " << filename;
		char fn[256];
		sprintf(fn, "images\\%s", filename);
		_imagearrays->back()->addImage(fn);
	};
	
	/*toolbar*/ 
	void addToolbar(const char*name) {	mframe->addToolbar(name);}
	void setToolbarImageArray(int i) {	mframe->lastToolbar()->setImageArray((*_imagearrays)[i]);}
	void addToolbarIcon(const char*caption, const char*command, int image) 
	{
		mframe->lastToolbar()->addIcon(image, caption, command);
	};
	
	/*window*/ 
	void positionWindow(int x, int y) { mframe->position(x,y); };
	void sizeWindow(int w, int h) { mframe->resize(w,h); };

	void addViewport()
	{
		Viewports::RequestObj r;
		Application::app()->requestModule(&r);

		mframe->addViewport(); 
		r.module()->manageViewport(mframe->viewport());
	};

private:
	MainFrame* mframe;
	std::vector<ImageArray*> *_imagearrays;
};
//
// UI TEST
//
struct test
{
static void ui()
{
	ParameterMap dt;

	ptui::Win *win = new ptui::Win(200,200,200,200);
	win->setParameterMap(&dt);
	win->newPanel();
	win->panel()->columns(1);
	win->panel()->newRow();
	win->panel()->newFloatSlider();

	win->show();
};
};
//
// UIManager
//
UIManager::UIManager(ptui::Interactor *i, MainFrame *mainframe)
{
	PTTRACE("UIManager::UIManager");
#define ASK		0
#define EXCLAIM 1
#define INFO	2

	m_mainframe = mainframe;
	m_interactor = i;
	uiman::_instance = this;

	loadUI();
	restoreUIState(0);
	
	if (uiman::cmdpipe())
	{
		uiman::cmdpipe()->addFunc("UI.buildWindow", boost::bind(&UIManager::buildWindowFromScript, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.showWindow", boost::bind(&UIManager::showWindow, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.hideWindow", boost::bind(&UIManager::hideWindow, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.appendText", boost::bind(&UIManager::appendText, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.clearText", boost::bind(&UIManager::clearText, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.saveText", boost::bind(&UIManager::saveText, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.getText", boost::bind(&UIManager::getText, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.loadText", boost::bind(&UIManager::loadText, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.updateUI", boost::bind(&UIManager::update, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.showTab", boost::bind(&UIManager::showTabbedPanel, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.messageBox", boost::bind(&UIManager::_messageBox, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.inputBox", boost::bind(&UIManager::_inputBox, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.redrawViewport", boost::bind(&UIManager::redrawViewport, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.updateToolbar", boost::bind(&UIManager::updateToolbarState, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.updateObjectsTree", boost::bind(&UIManager::updateObjectsTree, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.uitest", &test::ui, "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.panelPointer", boost::bind(&UIManager::panelPointer, this), "", 0, CPF_CALL_ONLY);
		uiman::cmdpipe()->addFunc("UI.objTreeToggle", boost::bind(&MainFrame::toggleObjbar, m_mainframe), "", 0, CPF_CALL_ONLY);
	}
	uiman::_bhandler = new ptl::BranchHandler("UI", &UIManager::readUIBranch, &UIManager::writeUIBranch);	

	ptui::setMainWindow(m_mainframe);
}
//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
UIManager::~UIManager()
{
	storeUIState(0);
}
//
//
//
bool UIManager::writeUIBranch(pt::datatree::Branch *b)
{
	//save some layout options in the project file
	MainFrame *mf = uiman::_instance->mainframe();
	b->addNode("objspos", mf->_objswidth);
	b->addNode("objsexpanded", mf->_objsexpanded);
	
#ifdef POINTOOLS_VIEW_PRO
	b->addNode("animpos", mf->_animpos);
	b->addNode("gr_ppu", mf->_timeline->pixelsPerUnit());
	b->addNode("gr_voff", mf->_timeline->graphOffset());
	b->addNode("gr_ppf", mf->_timeline->pxperFrame());
	b->addNode("gr_toff", mf->_timeline->timeOffset());
#endif
	return true;
}
//
//
//
bool UIManager::readUIBranch(const pt::datatree::Branch *b)
{
	//read some layout options back from the file
	MainFrame *mf = uiman::_instance->mainframe();
	b->getNode("objspos", mf->_objswidth);
	b->getNode("objsexpanded", mf->_objsexpanded);
#ifdef POINTOOLS_VIEW_PRO
	b->getNode("animpos", mf->_animpos);
	float fval;
	
	if (b->getNode("gr_ppu", fval))
	{
		mf->_timeline->pixelsPerUnit(fval);

		b->getNode("gr_voff", fval);
		mf->_timeline->graphOffset(fval);

		b->getNode("gr_ppf", fval);
		mf->_timeline->pxperFrame(fval);

		b->getNode("gr_toff", fval);
		mf->_timeline->timeOffset(fval);
		
		mf->_timeline->redraw();
	}
#endif
	mf->relayout();
	mf->redraw();
	return true;
}
//----------------------------------------------------------------------------------------
// Update Button State
//----------------------------------------------------------------------------------------
void UIManager::updateToolbarState()
{
	PTTRACE("UIManager::updateToolbarState");

	typedef ptWidget<fltk::Button> Icon;

	/* go through toolbar updating state of buttons	*/ 
	/* according to the setting they represent		*/ 
	/* only valid for state buttons					*/ 
	int toolbars = m_mainframe->numToolbars();
	int i,j;
	int nat = m_mainframe->numAnimToolbars();

	for (i=0; i<toolbars+nat; i++)
	{
#ifdef POINTOOLS_VIEW_PRO
		ptui::ToolBar *toolbar = i < toolbars ? m_mainframe->toolbar(i) : m_mainframe->animationToolbar(i-toolbars);
#else
		if (i >= toolbars) continue;

		ptui::ToolBar *toolbar = m_mainframe->toolbar(i);
#endif

		for (j=0; j<toolbar->children(); j++)
		{
			Icon *icon = (Icon*)toolbar->child(j);
			/*check for setting command*/ 
			const char*cmd = icon->cmdstring;
		
			/* a Hack to get Viewing mode to behave like integer value */ 
			/* THIS IS LANGUAGE AND ICON ORDER DEPENDENT */ 
			if (j==0 && icon->label() && toolbar->children() >= 5 && strcmp("View", icon->label())==0)
			{
				int active_mode = 0;

				switch(m_mainframe->viewport()->viewport->nav_mode())
				{
					case VP_ZOOM: active_mode=1; break;
					case VP_PAN: active_mode=2; break;
					case VP_ROTATE: active_mode=3; break;
					case VP_FREE: active_mode=4; break;	
				}
				if (active_mode)
				{
					toolbar->clearGroup();
					toolbar->child(active_mode)->set_flag(fltk::VALUE);
				}
				toolbar->redraw();
				continue;
			}
			
			if (j==0)
			{
				if (!icon->user_data()) icon->clear_value(); 			
				continue;
			}

			if (cmd[0] == '!')
			{
				++cmd;
				int thisval, val;

				/*//integer value*/ 
				if (cmd[0] == 'i')
				{
					++cmd;
					int thisval = cmd[0] - '0';
				
					++cmd;
					if (cmd[0] == 'R') ++cmd;
					
					if ((getVal(cmd, val) || getSetting(cmd, val)) && val == thisval)
						icon->set_flag(fltk::VALUE);
					else icon->clear_flag(fltk::VALUE);
				}
				else
				{
					/*boolean value*/ 
					bool val;
					if (cmd[0] == 'R') ++cmd;

					if (getVal(cmd, val) || getSetting(cmd, val))
					{
						if (val) icon->set_flag(fltk::VALUE);
						else icon->clear_flag(fltk::VALUE);
					}
				}		
			}
			/*cancel modes*/ 
			else if (cmd[0] != 'G' && cmd[0] != '#')
			{
				icon->clear_flag(fltk::VALUE);
			}
			toolbar->redraw();
		}
	}
	/* THIS IS HEAVILY DEPENDENT ON ICON ? TOOLBAR ORDER AND GENERALLY NOT A GOOD THING! */ 
	if (nat >= 2)
	{
		m_mainframe->animationToolbar(1)->clearGroup();

		m_mainframe->animationToolbar(1)->child(3)->clear_flag(fltk::VALUE); 
		m_mainframe->animationToolbar(1)->child(2)->clear_flag(fltk::VALUE); 
		m_mainframe->animationToolbar(1)->child(1)->clear_flag(fltk::VALUE);

		switch(m_mainframe->timeline()->navMode())
		{
			case ptui::Timeline::FullNavMode: 
				m_mainframe->animationToolbar(1)->child(3)->set_flag(fltk::VALUE); break;
			case ptui::Timeline::ZoomNavMode: 
				m_mainframe->animationToolbar(1)->child(2)->set_flag(fltk::VALUE); break;
			case ptui::Timeline::PanNavMode: 
				m_mainframe->animationToolbar(1)->child(1)->set_flag(fltk::VALUE); break;
		};
		m_mainframe->animationToolbar(1)->redraw();
	}
	m_mainframe->redraw();
	fltk::flush();
}
//----------------------------------------------------------------------------------------
// Update Button State
//----------------------------------------------------------------------------------------
void UIManager::redrawViewport()
{
	uiman::cmdpipe()->call("_ptRedraw");
	fltk::flush();
}

void UIManager::updateObjectsTree()
{
	if (ObjectsWin::instance()->populate())
		ObjectsWin::instance()->redraw();
#ifdef POINTOOLS_VIEW_PRO
	if (AnimObjectsWin::instance()->populate())
		AnimObjectsWin::instance()->redraw();
#endif
}
//----------------------------------------------------------------------------------------
// Load UI
//----------------------------------------------------------------------------------------
bool UIManager::loadUI()
{
	PTTRACE("UIManager::loadUI");

	_action a(mainframe(), &m_imagearrays);
	
	char filepath[PT_MAXPATH];
	sprintf(filepath, "%s\\%s", ptapp::apppathA(), "config\\startup.cfg");

	PTTRACEOUT << "Parsing file...";
	if (!uiparser::parseFile(filepath, &a))
	{
		fltk::alert("Error in startup.cfg: %s", uiparser::getError());
		m_error = true;
		return false;
	}
	PTTRACEOUT << "done parsing";

	a.addViewport();
	m_error = false;

	return true;
}
//
//
//
bool UIManager::storeUI()
{
	/*ammend configuration file*/ 

	return true;
}
//
// Commandpipe functions
//
void UIManager::buildWindowFromScript()
{
	PTTRACE("UIManager::buildWindowFromScript");

	ptui::Win::setInteractor(m_interactor);
	unsigned int pl=0;
	pt::String filename;
	pt::ParameterList *args = uiman::cmdpipe()->getCallParameters();

	char filepath[MAX_PATH];

	if (args->get(pl, filename)==-1)
	{
		pt::ParameterMap *dt = (pt::ParameterMap*)pl;
		//try
		{
			sprintf(filepath, "%s\\%s", ptapp::apppathA(), filename.c_str());
			
			PTTRACEOUT << "building script " << filepath;
			if (_access(filepath, 0)==0)
			{
				PTTRACE("Building Script"); 
				std::ifstream ifs(filepath);
				PTTRACEOUT << "created ifs"; 
			
			//	ptui::Splash::setProgressMessage(filepath);
				
				char _error [256];
				ptuiParser parser(&ifs);

				try 
				{
					PTTRACE("try parsing file"); 
					parser.file(dt);
				} 
				catch (ScanException &scex)
				{
					strcpy(_error, ((std::string)scex).c_str());
					PTTRACEOUT << "Scan Error " << _error;
					
					fltk::alert("Scan Error in interface script file %s \n%s", filepath, _error);
					return;
				}
				catch (ParseException &pex)
				{
					strcpy(_error, ((std::string)pex).c_str());
					PTTRACEOUT << "Parse Error " << _error;

					fltk::alert("Parse Error in interface script file %s \n%s", filepath, _error);
					return;
				}
				PTTRACEOUT << parser._windows.size() << " windows created:";

				for (uint i=0; i<parser._windows.size(); i++)
				{
					parser._windows[i]->hide();
					parser._windows[i]->updatePanels();
					PTTRACEOUT << i << ": " << (parser._identifiers[i].c_str());

					m_windows.insert(WINMAP::value_type(parser._identifiers[i], parser._windows[i]));
				}
			}
			else
			{
				char mess[128];
				sprintf(mess, "Can't access interface script %s\n", filename.c_str());
				ptui::alertBox("Script Error",mess);		
				return;
			}
		}
	}
	else
	{
		debugAssertM(0,"Wrong parameters provided to buildWindowFromScript");
	}
}
//
// build panels - this is a direct function 
//
int UIManager::buildPanelsFromScript(const char* filename, ParameterMap *dt, std::vector<ptui::Panel*> &panels)
{
	PTTRACE("UIManager::buildPanelsFromScript");

	/*pass filename arg through result stack*/ 
	char filepath[260];

	//try
	{
		sprintf(filepath, "%s\\%s", ptapp::apppathA(), filename);
		
		PTTRACEOUT << "building script " << filepath;
		if (_access(filepath, 0)==0)
		{
			PTTRACE("Building Script"); 
			std::ifstream ifs(filepath);
			PTTRACEOUT << "created ifs"; 
					
			char _error [1024];
			ptuiParser parser(&ifs);

			try 
			{
				PTTRACE("try parsing file"); 
				parser.file(dt);
			} 
			catch (ScanException &scex)
			{
				strcpy(_error, ((std::string)scex).c_str());
				PTTRACEOUT << "Scan Error " << _error;
				
				fltk::alert("Scan Error in interface script file %s \n%s", filepath, _error);
			}
			catch (ParseException &pex)
			{
				strcpy(_error, ((std::string)pex).c_str());
				PTTRACEOUT << "Parse Error " << _error;

				fltk::alert("Parse Error in interface script file %s \n%s", filepath, _error);
			}
			PTTRACEOUT << parser._panels.size() << " windows created:";

			for (uint i=0; i<parser._panels.size(); i++)
			{
				parser._panels[i]->updatePanel();
				PTTRACEOUT << i << ": " << (parser._identifiers[i].c_str());

				panels.push_back(parser._panels[i]);
			}
			return parser._panels.size();
		}
		else
		{
			fltk::alert("Can't access interface script %s\n", filename);		
			return 0;
		}
	}
}
//
// show window
//
void UIManager::showWindow()
{
	PTTRACE("UIManager::showWindow");

	pt::String name;
	ParameterList *args = uiman::cmdpipe()->getCallParameters();

	if (args->get(name) == -1)
	{	
		PTTRACEOUT << "window = " << name.c_str();

		WINMAP::iterator it = m_windows.find(name.c_str());
		if (it != m_windows.end())
		{
			ptui::Win *win = it->second;
			/*is this a modal dialog?*/ 
			if (win->isDialog())
			{
				if (!win->visible())
				{
					// this is not perfect and will fail
					// when a parent has multiple children windows
					fltk::Window *parentwin = m_mainframe;

					static ptui::Win *lastwin = 0;
					if (lastwin && lastwin->visible())
						parentwin = lastwin;
					lastwin = win;
					
					win->panel()->commitPanel();
					win->show(parentwin);
					win->take_focus();
					win->exec(parentwin);

					if (_lastWindow)
					{
						WINMAP::iterator hit = m_windows.find(_lastWindow);
						if (hit != m_windows.end())
						{
							ptui::Win::close(hit->second->child(0), 0);
							_lastWindow = 0;
						}
					}
				}
				else return;
			}
			else
			{
				win->show(m_mainframe);
				win->take_focus();
			}
			_lastWindow = name;
		}
		PTTRACEOUT << name.c_str() << " <window id> invalid"; 
	}
	else 
	{
		PTTRACEOUT << "missing <window id>"; 
	}
}

//
// show window
//
void UIManager::hideWindow()
{
	PTTRACE("UIManager::hideWIndow");

	String name;
	ParameterList *args = uiman::cmdpipe()->getCallParameters();

	if (args->get(name) == -1)
	{	
		WINMAP::iterator it = m_windows.find(name.c_str());
		if (it != m_windows.end())
		{
			ptui::Win::close(it->second->child(0), 0);//->destroy();
			_lastWindow = 0;
		}
	}
}
//
// update widget
//
void UIManager::update()
{
	PTTRACE("UIManager::updateWidget");
	
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;
	char wid[64];

	if (parseCmdPipeSpec(win, pan, wid))
	{
		pan->updateWidget(wid);
	}
	else if (pan) pan->updatePanel();
	else if (win) win->updatePanels();
}
//
// show tab
//
void UIManager::showTabbedPanel()
{
	PTTRACE("UIManager::showTabbedPanel");
	
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;
	char wid[64];

	parseCmdPipeSpec(win, pan, wid);

	if (win && pan)
	{
		win->panelGroup()->showPanel(pan);
	}
}
//
//
//
void UIManager::panelPointer()
{
	PTTRACE("UIManager::panelPointer");
	
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;

	String spec;
	ParameterList *args = uiman::cmdpipe()->getCallParameters();

	if (args->get(spec) == -1)
	{
		parseSpec(spec.c_str(), win, pan);
		
		CmdResult r; 
		r.setPtr(pan);
		uiman::cmdpipe()->pushResult(r);
	}
}
//
// Parse spec
//
bool UIManager::parseSpec(const char*spec, ptui::Win *&w, ptui::Panel *&p)
{
	/*parse for tokens*/ 
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(".");
	std::string _spec(spec);
	tokenizer tokens(_spec, sep);

	tokenizer::iterator tok_iter = tokens.begin();
	
	/*window name*/ 
	WINMAP::iterator it = m_windows.find(*tok_iter);
	if (it != m_windows.end())
	{
		w = it->second;
		/*find panel*/ 
		if (++tok_iter != tokens.end())
		{
			p = w->panel((*tok_iter).c_str());
			
			if (!p) return false;
			else return true;
		}
	}
	return false;
}
//
// Parse spec
//
bool UIManager::parseSpec(const char*spec, ptui::Win *&w, ptui::Panel *&p, char *wid)
{
	/*parse for tokens*/ 
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(".");
	std::string _spec(spec);
	tokenizer tokens(_spec, sep);

	tokenizer::iterator tok_iter = tokens.begin();
	
	/*window name*/ 
	WINMAP::iterator it = m_windows.find(*tok_iter);
	if (it != m_windows.end())
	{
		w = it->second;
		/*find panel*/ 
		if (++tok_iter != tokens.end())
		{
			p = w->panel((*tok_iter).c_str());
			
			if (!p) return false;

			if (++tok_iter != tokens.end())
			{
				strncpy(wid, (*tok_iter).c_str(), 64);
				return true;
			}
		}
	}
	return false;
}
//
//
//
bool UIManager::parseCmdPipeSpec(ptui::Win *&w, ptui::Panel *&p, char *wid)
{
	String spec;
	ParameterList *args = uiman::cmdpipe()->getCallParameters();

	if (args->get(spec) == -1)
	{	
		return parseSpec(spec.c_str(), w, p, wid);
	}
	return false;
}
//
// text output 
//
void UIManager::appendText()
{
	/*pass filename arg through result stack*/ 
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;
	char wid[64];

	if (parseCmdPipeSpec(win, pan, wid))
	{
		String text;
		ParameterList *args = uiman::cmdpipe()->getCallParameters();
	
		if (args->getAt(1, text) && text.length())
		{
			pan->appendText(wid, text.c_str());
		}
	}
}
void UIManager::clearText()
{
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;
	char wid[64];

	if (parseCmdPipeSpec(win, pan, wid))
	{
		pan->clearText(wid);
	}
}
void UIManager::getText()
{
	/*copy text buffer into character pointer*/ 
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;
	char wid[64];

	if (parseCmdPipeSpec(win, pan, wid))
	{
		void* buffer;
		ParameterList *args = uiman::cmdpipe()->getCallParameters();

		if (args->getAt(1, buffer))
		{
			pan->getText(wid, (char*)buffer);
		}
		else debugAssertM(0, "Wrong arguments supplied");
	}
}
void UIManager::saveText()
{
	/*pass filename arg through result stack*/ 
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;
	char wid[64];
	if (parseCmdPipeSpec(win, pan, wid))
	{
		ParameterList pl = ParameterList::create(String("txt"), String("Text File"), String(""), true);
		
		const char* filename;
		uiman::cmdpipe()->call("acqFilename", &pl);

		CmdResult res;
		uiman::cmdpipe()->popResult(res);

		if (res.getVal(filename))
		{
			char fn[260];
			strcpy(fn, filename);

			int len = strlen(fn);
			if (strcmp(&fn[len-5], ".txt") != 0
				&& strcmp(&fn[len-5], ".TXT") != 0)
			{
				strcpy(&fn[len], ".txt");
			}
			pan->saveTextFile(wid, fn);
		}
	}
}
//
// load text
//
void UIManager::loadText()
{
	/*pass filename arg through result stack*/ 
	ptui::Win *win = 0;
	ptui::Panel *pan = 0;
	char wid[64];

	if (parseCmdPipeSpec(win, pan, wid))
	{
		String filename;
		ParameterList *pl = uiman::cmdpipe()->getCallParameters();

		if (pl->getAt(1,filename))
		{
			pan->openTextFile(wid, filename.c_str());
		}
	}
}
bool UIManager::setSetting(const char* setting, bool value)
{
	/* find window, panel, variable */ 
	char var[64];
	ptui::Panel *panel=0;
	ptui::Win *win=0;

	if (parseSpec(setting, win, panel, var))
	{
		if (panel->parameterMap()->set(var, value))
		{
			/* update widget from parameter - because it is read back in callback*/ 
			panel->updateWidget(var);
			fltk::Widget *wid = panel->getWidget(var);
			if (wid) wid->do_callback();
		}
	}
	return false;
}
bool UIManager::setSetting(const char* setting, int value)
{
	/* find window, panel, variable */ 
	char var[32];
	ptui::Panel *panel;
	ptui::Win *win;

	if (parseSpec(setting, win, panel, var))
	{
		if(panel->parameterMap()->set(var, value))
		{
			/* update widget from parameter - because it is read back in callback*/ 
			panel->updateWidget(var);
			fltk::Widget *wid = panel->getWidget(var);
			if (wid) wid->do_callback();			
		}
	}
	return false;
}
bool UIManager::getSetting(const char* setting, int &value)
{
	/* find window, panel, variable */ 
	char var[32];
	ptui::Panel *panel;
	ptui::Win *win;

	if (parseSpec(setting, win, panel, var))
	{
		return panel->parameterMap()->get(var, value);
	}
	return false;
}
bool UIManager::getSetting(const char* setting, bool &value)
{
	/* find window, panel, variable */ 
	char var[64];
	ptui::Panel *panel=0;
	ptui::Win *win=0;

	if (parseSpec(setting, win, panel, var))
	{
		return panel->parameterMap()->get(var, value);
	}
	return false;
}
bool UIManager::toggleSetting(const char*setting, bool &newval)
{
	if (getSetting(setting, newval))
	{
		newval = !newval;
		setSetting(setting, newval);
		return true;
	}
	return false;
}
bool UIManager::getVal(const char *var, bool &val)
{
	PTTRACE("UIManager::getVal");
	return SettingsWin::instance()->getSetting(var, val);
}
bool UIManager::getVal(const char *var, int &val)
{
	PTTRACE("UIManager::getVal");
	return SettingsWin::instance()->getSetting(var, val);
}
bool UIManager::getVal(const char *var, float &val)
{
	PTTRACE("UIManager::getVal : ASSERT");
//	return SettingsWin::instance()->getSetting(var, val);
	return false;
}
//
// Command Pipe MessageBox
//
void UIManager::_messageBox()
{
	PTTRACE("UIManager::_messageBox");

	/*generic message box*/ 

	//arguments
	// title
	// type
	// responses
	String title, response, message;
	std::vector<const char*> responses;
	static char sresponses[512];
	int type;

	ParameterList *pl = uiman::cmdpipe()->getCallParameters();
	if (pl->get(title, message, type ) == -1)
	{	
		for (int i=3; i<pl->size(); i++)
		{
			if (pl->getAt(i, response))
			{
				strcpy(&sresponses[(i-3)*32], response.c_str());
				responses.push_back(&sresponses[(i-3)*32]);
			}
		}
	}
	else debugAssertM(0, "Type mismatch in parameter list");

	CmdResult res(messageBox(type, title.c_str(), message.c_str(), responses));
	uiman::cmdpipe()->pushResult(res);
}
//
// alert Box
//
void UIManager::alertBox(const char* title, const char *message)
{
	ptui::alertBox(title, message);
}
//
// prompt box
//
int UIManager::validateBox(const char*title, const char *message)
{
	return ptui::validateBox(title, message);
}
//
// messageBox
//
int UIManager::messageBox(int type, const char* title, const char* message, const std::vector<const char*> &_responses)
{
	const char* responses [10];
	for (int i=0; i<_responses.size(); i++) responses[i] = _responses[i];
	return ptui::messageBox((ptui::MessageBoxType)type, title, message, _responses.size(), responses);
}
//
// InputBox
//
void UIManager::_inputBox()
{
	PTTRACE("UIManager::_inputBox");

	/*generic message box*/ 

	//arguments
	// title
	// type
	// responses
	String title, message;

	ParameterList *args = uiman::cmdpipe()->getCallParameters();
	if (args->get(message, title) ==-1)
	{
		const char *ret = 0;
		ret = inputBox(title.c_str(), message.c_str());

		if (ret)
		{
			CmdResult res(ret);
			uiman::cmdpipe()->pushResult(res);
		}
	}
	else debugAssertM(0, "Type mismatch in parameter list");
}
//
// Input Box
//
const char *UIManager::inputBox(const char *title, const char* message)
{
	return ptui::inputBox(title, message);
}
//
// update title
//
void UIManager::updateTitle()
{
	/*get application name and project name*/ 
	static char caption[260];

	sprintf(caption, "%s  :  %s %s", ptapp::applicationString(), ptl::Project::project()->filename(), 
		ptl::Project::project()->modified() ? "*" : " ");
	mainframe()->label(caption);

}
int UIManager::storeUIState(unsigned char* data)
{
	PTTRACE("UIManager::storeUIState");

	memrw w;
	w.reset();
	data = new unsigned char[200];
	w.set_wdata(data);
	
	int version = 1;

	w.write(version);

	/*mainframe size*/ 
	w.write(mainframe()->storeX());
	w.write(mainframe()->storeY());
	w.write(mainframe()->storeW());
	w.write(mainframe()->storeH());

	/*toolbar states*/ 
	w.write(mainframe()->numToolbars());
	for (int i=0; i<mainframe()->numToolbars(); i++)
	{
		bool c = mainframe()->toolbar(i)->collapsed();
		w.write(c);
	}

	/*UI data - window pos etc */ 
	pt::String uf;
	char path[MAX_PATH];
	ptapp::env()->get("user_folder", uf);
	strcpy(path, uf.c_str());
	::PathAppend(path, "ptview_ui_state.cfg");
	ptfs::FilePath fpui(path);
		
	ptfs::FileHandle fh = ptfs::IO::openForWrite(fpui);
	if (ptfs::IO::validHandle(fh))
	{
		ptfs::IO::writeBytes(fh, w.wdata, w.byte_size);
		ptfs::IO::close(fh);
	}
	w.release();
	return w.byte_size;
}
void UIManager::restoreUIState(unsigned char* data)
{
	PTTRACE("UIManager::storeUIState");

	/*UI data - window pos etc */ 
	pt::String uf;
	char path[MAX_PATH];
	ptapp::env()->get("user_folder", uf);
	strcpy(path, uf.c_str());
	::PathAppend(path, "ptview_ui_state.cfg");
	ptfs::FilePath fpui(path);
		
	ptfs::FileHandle fh = ptfs::IO::openForRead(fpui);

	unsigned char buffer[1024];
	memrw r;
	r.set_rdata(buffer);
	r.reset();

	if (ptfs::IO::validHandle(fh))
	{
		ptfs::IO::readBytes(fh, buffer, ptfs::IO::getFileSize(fh));
		ptfs::IO::close(fh);
	}
	else return;

	int version;
	int pos = 0;
	r.read(version, pos);

	if (version == 1)
	{
		int x, y, w, h;
		r.read(x, pos);
		r.read(y, pos);
		r.read(w, pos);
		r.read(h, pos);
#ifdef WIN32
		int mx = ::GetSystemMetrics(SM_CXMAXIMIZED);
		int my = ::GetSystemMetrics(SM_CYMAXIMIZED);
		int cap = ::GetSystemMetrics(SM_CYCAPTION);	
#endif
		if (mx <= w || mx <= y)
		{
			mainframe()->resize(mx,my-cap);
			mainframe()->position(10,cap);
			mainframe()->show();
		}
		else
		{
			if (w < 50){ w = 500; h = 500; }
			if (h < 50){ h = 500; w = 500; }
			if (x < 0) x = 0;
			if (y < cap) y = cap;
			mainframe()->resize(x,y,w,h);
		}

		int nt; bool c;
		r.read(nt, pos);
		for (int i=0; i<nt; i++)
		{
			r.read(c, pos);
			if (mainframe()->numToolbars() > i)
				mainframe()->toolbar(i)->collapsed(c);
		}
	}
}
//
// store User State to File
//
void UIManager::storeUserState() const
{


}
//
// restore User State from File
//
void UIManager::restoreUserState()
{

}
