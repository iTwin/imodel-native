/*--------------------------------------------------------------------------*/ 
/*	Pointools Tool class implementation										*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 26 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/

#include <pttool/Tool.h>
#include <ptcmdppe/CommandPipe.h>
#include <ptvpm/Viewports.h>
#include <ptcmdppe/interact.h>
#include <pt/trace.h>
#include <ptappdll/ptapp.h>

#include <pt/debugassert.h>

using namespace ptapp;
using namespace pt;

namespace ___pttool
{
ModulesManager *_app  = 0;
CommandPipe *_cmdpipe  = 0;
Viewports *_vpm = 0;
bool _vlock = false;
static unsigned short licstatus = Tool::LicNotRequired;
static unsigned short *licstatusptr = &licstatus;
static unsigned int check = ((unsigned int)rand()) & ~32;
}
using namespace ___pttool;

/*red herrings */ 
void *Tool::licenseInfo() const
{
	return (this + 44);
}
bool Tool::isLicenseValid() const
{
	return true;
}
/* actual license info */ 
void *Tool::versionInfo() const
{	
	static VersionInfo v;
	v.version [0] = 1;
	v.version [1] = 0;
	v.version [2] = 0;
	v.version [3] = 0;

	v.execs_left = -1;
	v.days_left = -1;
	v.check = &check;
	v.license_status = &licstatusptr;

	return &v;
}
//
void Tool::drop()
{
//	popViewLock();
	_cmdpipe->pop();
}
void Tool::setCursor(uint cursor)
{
	_cmdpipe->overideCursor(cursor);
}
//
void Tool::requestModule(pt::RequestObj *r)
{
	_app->requestModule(r);
}
//
bool Tool::initialize()
{
	if (!_app)
	{
		_app = ptapp::application();
	}
   	CommandPipe::RequestObj r;
    requestModule(&r);
    _cmdpipe = r.module();

   	Viewports::RequestObj rv;
    requestModule(&rv);
    _vpm = rv.module();  

	char *dummy = "Licensed";

	return true;
}
//
bool Tool::registerEventCallback(ptapp::eventCB cb)
{
	debugAssertM(_cmdpipe, "Attempting to register function. pttool::Tool has not been initialized");
	if (_cmdpipe) _cmdpipe->addEventCB(cb);
	return true;
}
//
bool Tool::registerCmd(const std::string &cmd, ptapp::functionCB cb, uint flags)
{
	debugAssertM(_cmdpipe, "Attempting to register function. pttool::Tool has not been initialized");
	if (_cmdpipe) _cmdpipe->addFunc(cmd, cb, " ", 0, CPF_CALL_ONLY | flags);
	return true;
}
//
bool Tool::registerEventCmd(const std::string &cmd, ptapp::eventCB cb, uint eventmask, const char *prompt, uint cursor, uint flags)
{
	debugAssertM(_cmdpipe, "Attempting to register function. pttool::Tool has not been initialized");
	if (_cmdpipe) _cmdpipe->addEventFunc(cmd, cb, " ", eventmask, prompt, flags, cursor);
	return true;
}
//																				
bool Tool::registerScript(const std::string &name, const std::string &script, const std::string &prompts, ptapp::eventCB cb, uint flags)
{
	debugAssertM(_cmdpipe, "Attempting to register script. pttool::Tool has not been initialized");
	if (_cmdpipe) _cmdpipe->addScript(name, script, prompts, cb, flags);
	return true;
}
//
bool Tool::peepResult(int &i)
{ 
	CmdResult r;
	_cmdpipe->peepResult(r);
	return r.getVal(i);
}
//
void Tool::pushResult(const int &i)
{
	CmdResult r(i);
	_cmdpipe->pushResult(r);
}
//
bool Tool::popResult(int &i)
{
	CmdResult r;
	_cmdpipe->popResult(r);
	return r.getVal(i);
}
//																				
bool Tool::peepResult(bool &b)
{
	CmdResult r;
	_cmdpipe->peepResult(r);
	return r.getVal(b);
}
void Tool::pushResult(const bool &b)
{
	CmdResult r;
	r.setVal(b);
	_cmdpipe->pushResult(r);
}
bool Tool::popResult(bool &b)
{
	CmdResult r;
	_cmdpipe->popResult(r);
	return r.getVal(b);
}	
//																					
bool Tool::peepResult(float &f)
{
	CmdResult r;
	_cmdpipe->peepResult(r);
	return r.getVal(f);
}
void Tool::pushResult(const float &f)
{
	CmdResult r(f);
	_cmdpipe->pushResult(r);
}
bool Tool::popResult(float &f)
{
	CmdResult r;
	_cmdpipe->popResult(r);
	return r.getVal(f);
}
//																						
bool Tool::peepResult(pt::vector3 &v)
{
	CmdResult r;
	_cmdpipe->peepResult(r);
	return r.getVal(v);
}
void Tool::pushResult(const pt::vector3 &v)
{
	CmdResult r((float*)&v);
	_cmdpipe->pushResult(r);
}
bool Tool::popResult(pt::vector3 &v)
{
	CmdResult r;
	_cmdpipe->popResult(r);
	return r.getVal(v);
}
//																				
bool Tool::peepResult(void *&ptr)
{
	CmdResult r;
	_cmdpipe->peepResult(r);
	return r.getPtr(ptr);
}
void Tool::pushResult(void *ptr)
{
	CmdResult r;
	r.setPtr(ptr);
	_cmdpipe->pushResult(r);
}
bool Tool::popResult(void *&ptr)
{
	CmdResult r;
	_cmdpipe->popResult(r);
	return r.getPtr(ptr);
}
//
// args
//
ParameterList *Tool::getCallParameters()
{ 
	return _cmdpipe->getCallParameters();
}
//
/*front buffer draw*/ 
void Tool::begin3dDraw()
{
	_vpm->makeCurrent();
}
//
void Tool::beginPixelDraw()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	_vpm->makeCurrent();
	GLdouble l = _vpm->camera()->getViewportLeft();
	GLdouble r = _vpm->camera()->getViewportRight();
	GLdouble t = _vpm->camera()->getViewportTop();
	GLdouble b = _vpm->camera()->getViewportBottom();

	/*setup ortho*/ 
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(l,r, b, t);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}
//
void Tool::beginXORDraw()
{
	glEnable(GL_COLOR_LOGIC_OP);
	
	glDrawBuffer(GL_FRONT);
	glLogicOp(GL_XOR);
}
//
void Tool::endXORDraw()
{
	glDisable(GL_COLOR_LOGIC_OP);
	glDrawBuffer(GL_BACK);
}
//
void Tool::endPixelDraw()
{
	glFlush();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
//
void Tool::redrawViewport()
{
	_vpm->redrawCurrent();
}
//
void Tool::dynamicRedrawViewport()
{
	_vpm->dynamicRedrawCurrent();
}
//
void Tool::refreshViewport()
{
	_vpm->refreshCurrent();
}
//
ptgl::Camera*Tool::camera()
{
	return _vpm->camera();
}
//
// UI
//		
void* Tool::panelPointer(const char *panel)
{
	ParameterList args = ParameterList::create(String(panel));
	_cmdpipe->call("UI.panelPointer", &args);	

	void* ptr;
	popResult(ptr);
	return ptr;
}
//
void Tool::buildWindow(const char *script, pt::ParameterMap *pm)
{
	std::cout << "building interface from " << script << std::endl;
	ParameterList args = ParameterList::create((void*)pm, String(script));
	_cmdpipe->call("UI.buildWindow", &args);
}
//
void Tool::showWindow(const char*win)
{
	ParameterList args = ParameterList::create(String(win));
	_cmdpipe->call("UI.showWindow", &args);
}
//
void Tool::hideWindow(const char*win)
{
	ParameterList args = ParameterList::create(String(win));
 	_cmdpipe->call("UI.hideWindow", &args);
}
//
void Tool::updateUI(const char*wid)
{
	ParameterList args = ParameterList::create(String(wid));
	_cmdpipe->call("UI.updateUI", &args);
}
//
void Tool::showTabbedPanel(const char*wid)
{
	ParameterList args = ParameterList::create(String(wid));
	_cmdpipe->call("UI.showTab", &args);
}
//
void Tool::appendText(const char*wid, const char *text)
{
	ParameterList args = ParameterList::create(String(wid), String(text));
	_cmdpipe->call("UI.appendText", &args);
}
//
void Tool::saveText(const char*wid)
{
	ParameterList args = ParameterList::create(String(wid));
	_cmdpipe->call("UI.saveText", &args);
}
//
void Tool::loadText(const char*wid, const char *filename)
{
	ParameterList args = ParameterList::create(String(wid), String(filename));;
	_cmdpipe->call("UI.loadText", &args);
}
//
void Tool::clearText(const char*wid)
{
	ParameterList args = ParameterList::create(String(wid));
	_cmdpipe->call("UI.clearText", &args);
}
//
void Tool::getText(const char *wid, char *buffer)
{
	void *buff = (void*)buffer;
	ParameterList args = ParameterList::create(String(wid), buff);
	_cmdpipe->call("UI.getText", &args);	
}
//
void Tool::pushViewLockFull()
{
	PTTRACEOUT << "TOOL: pushViewLockFull";

	_vlock = true;
	_vpm->pushViewLockFull();
}
//
void Tool::pushViewLockHalf()
{
	PTTRACEOUT << "TOOL: pushViewLockHalf";
	
	_vlock = true;
	_vpm->pushViewLockHalf();
}
//
void Tool::popViewLock()
{
	PTTRACEOUT << "TOOL: popViewLock";
	if (_vlock)	_vpm->popViewLock();
	_vlock = false;
}
//
// ui stuffs
//
bool Tool::okcancelMessage(const char*title, const char* message)
{
	/*message */ 
	ParameterList pl =  ParameterList::create(String(title), String(message), 2, String("Cancel"), String("Ok"));
	_cmdpipe->call("UI.messageBox", &pl);

	CmdResult res;
	_cmdpipe->popResult(res);
	
	int ret;
	res.getVal(ret);

	return !ret;
}
bool Tool::yesnoMessage(const char*title, const char* message)
{
	/*message */ 
	ParameterList pl =  ParameterList::create(String(title), String(message), 2, String("No"), String("Yes"));
	_cmdpipe->call("UI.messageBox", &pl);

	CmdResult res;
	_cmdpipe->popResult(res);
	
	int ret;
	res.getVal(ret);

	return ret;
}
void Tool::okMessage(const char*title, const char* message)
{
	/*message */ 
	ParameterList pl = ParameterList::create(String(title), String(message), 0, String("Ok"));
	_cmdpipe->call("UI.messageBox", &pl);
}
//
void Tool::errorMessage(const char*title, const char* message)
{
	/*message */ 
	ParameterList pl = ParameterList::create(String(title), String(message), 1, String("Ok"));
	_cmdpipe->call("UI.messageBox", &pl);
}
//
void Tool::callCmd(const char*cmd, pt::ParameterList *args)
{
	if (args) _cmdpipe->call(cmd, args);
	else _cmdpipe->call(cmd);
}
void Tool::pushCmd(const char *cmd)
{
	_cmdpipe->set(cmd);
}

const char* Tool::getOpenFilepath(const char *extension, const char *filetype, const char *initialpath)
{
	ParameterList pl = ParameterList::create(false, String(initialpath), String(extension), String(filetype));
	callCmd("UI.acqFilename", &pl);

	CmdResult res;
	_cmdpipe->popResult(res);

	const char*filename;
	if (res.getVal(filename))
		return filename;
	return 0;
}
const char* Tool::getSaveFilepath(const char *extension, const char *filetype, const char *initialpath)
{
	ParameterList pl = ParameterList::create(true, String(initialpath), String(extension), String(filetype));
	callCmd("UI.acqFilename", &pl);

	CmdResult res;
	_cmdpipe->popResult(res);

	const char*filename;
	if (res.getVal(filename))
		return filename;
	return 0;
}
