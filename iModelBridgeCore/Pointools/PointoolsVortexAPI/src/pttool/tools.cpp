/*--------------------------------------------------------------------------*/ 
/*	Pointools Tools application module class definition						*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 28 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#include <pt/typedefs.h>
#include <pttool/Tool.h>
#include <pttool/Tools.h>

#include <shlwapi.h>
#include "cxr_inc.h"

#include <boost/date_time/gregorian/gregorian.hpp>

//#import "ClientProtectorCOM.dll" no_namespace 

#include <ptui/ptWindow.h>
#include <ptui/ptGrid.h>
#include <ptui/ptMessage.h>

#include <ptcmdppe/commandpipe.h>
#include <ptui/ptuiprefs.h>

#define LIC_TRUE_NON_EXPIRY			100
#define LIC_TRUE_EXPIRY				101
#define LIC_TRUE_CODE_ACTIVATED		102
#define LIC_FALSE_ATTEMPT_DEFEAT	500
#define LIC_FALSE_REFUSE_ACTIVATE	501
#define LIC_FALSE_BAD_AUTH_CODE		502
#define LIC_FALSE_EXPIRED			503
#define LIC_FALSE_MISSING_MAIN_LF	504
#define LIC_FALSE_CODE_NOT_ACTIVATED		505
#define LIC_FALSE_COMMUNICATION_FAILURE		506
#define LIC_FALSE_SYSTEM_NOT_AUTHORIZED		507
#define LIC_FALSE_INTERNAL_FAILURE			508
#define LIC_FALSE_WAITING_FOR_CODE			509
#define LIC_FALSE_INITIALIZATION_FAILURE	550

//
// plugins manager
//
#define NUMSYM 2
typedef pt::Tool*	(*GETTOOL)(void);
typedef const char*	(*GETDESC)(void);

#define SYM1			"getTool"
#define SYM1MARGS       "v"
#define FUNC1			GETTOOL		
#define FUNC1IDENTIFIER	pt::Tool* getTool		

#define SYM2			"getDescription"
#define SYM2MARGS       "v"
#define FUNC2			GETDESC		
#define FUNC2IDENTIFIER	const char* getDescription

#include <ptapp/pluginsmanager.h>

static ptapp::PluginsManager s_plugins("getPTToolDescriptor");
//ICoClientProtectorPtr entrysys;

#undef GETTOOL


namespace __ptapp
{
	ptui::Win *_pluginsWin = 0;
	ptui::Panel *_pluginsPanel = 0;
	ptui::Grid *_pluginsGrid = 0;

	pt::Tool * _current=0;
	pt::ParameterMap _params;
};
using namespace pt;
using namespace ptapp;
using namespace __ptapp;

/* extern encrypted strings*/ 
extern const char* strLicLoc;
extern const char* strLicensed;
extern const char* strExecLimit;
extern const char* strDaysLimit;
extern const char* strExpired;
extern const char* strPluginLicensing;
extern const char* strActivateSuccess;
extern const char* strCodeInvalid;
extern const char* strUsesMessage;
extern const char* strTrialFinished;
extern const char* strCorrupt;
extern const char* strActivated;
extern const char* strLicense;
extern const char* strLicensePlugin;

//int initializeCOM()
//{
//	static bool initialized = false;
//	if (initialized) return 1;
//
//	initialized = true;
//
//	/*initialize COM*/ 
//	CoInitialize(NULL);
//
//	/* initialize and startup the ClientProtector COM server*/ 
//	CLSID clsID;
//
//	HRESULT hr; hr= CLSIDFromProgID(OLESTR("ClientProtectorCOM.CoClientProtector"), &clsID);
//	if(FAILED(hr))
//	{
//		return 0;
//	} 
//	entrysys.CreateInstance(clsID);
//	return 1;
//}
//
// licensing
//
int checkLicense(const char *filename, const char* localpwd, const char *globalpwd)
{
//	if (!initializeCOM()) return LIC_FALSE_INITIALIZATION_FAILURE;
//
//	char MainLicenseFileName[260];
//	char PluginLicenseFileName[260];
//	char licenseCfgFilepath[260];
//
//	sprintf(licenseCfgFilepath, "%s\\%s", ptapp::apppathA(), _CXR(strLicLoc).c_str());
//
//	/*get license location from config file*/ 
//	std::ifstream ifs;
//	ifs.open(licenseCfgFilepath, std::ios::in);
//	if (!ifs.is_open())	return LIC_FALSE_INITIALIZATION_FAILURE;
//
//	ifs.getline(MainLicenseFileName, 260, '\n');
//	::PathRemoveFileSpecA(MainLicenseFileName);
//	sprintf(PluginLicenseFileName, "%s\\pt%s.lic", MainLicenseFileName, filename);
//	ifs.close();
//
//	char MainLicenseFilePassword[256];
//	char GlobalAuthorizationCodePassword[256];
//
//	strncpy(MainLicenseFilePassword, _CXR(localpwd).c_str(), sizeof(MainLicenseFilePassword));
//	strncpy(GlobalAuthorizationCodePassword, _CXR(globalpwd).c_str(), sizeof(GlobalAuthorizationCodePassword));
//
//	int FingerPrintOptionsCode = 1164;
//	long return_code; 
//
//	/*check the license */ 
//	entrysys->StartUp(PluginLicenseFileName, MainLicenseFilePassword, GlobalAuthorizationCodePassword, FingerPrintOptionsCode, &return_code);
//
//	return return_code;
	return 0;
}
//
// act on license 
//
int handleLicense(int code)
{
	/*check license state*/ 
	/*switch (code)
	{
		case LIC_TRUE_NON_EXPIRY:
		case LIC_TRUE_CODE_ACTIVATED: 
			return Tool::LicLicensed;

		case LIC_TRUE_EXPIRY: 
			return Tool::LicEvaluation;

		case LIC_FALSE_ATTEMPT_DEFEAT:
			return Tool::LicCorrupt;

		case LIC_FALSE_REFUSE_ACTIVATE:
		case LIC_FALSE_BAD_AUTH_CODE:
			return Tool::LicInvalid;

		case LIC_FALSE_EXPIRED:
			return Tool::LicExpired;

		case LIC_FALSE_MISSING_MAIN_LF:
			return Tool::LicFileMissing;

		case LIC_FALSE_INTERNAL_FAILURE:
			return Tool::LicInternalError;

		case LIC_FALSE_WAITING_FOR_CODE:
			return Tool::LicInvalid;

		case LIC_FALSE_INITIALIZATION_FAILURE:
			return Tool::LicInternalError;

		default:
			return Tool::LicInvalidFile;
	}			*/
	return 0;
}
void getExpiryInfo(int &execs, int &days)
{
	//// days remaining
	//long expYear, expMonth, expDay, expHour, expMinute, execs_left;
	//short isExpireType;

	//entrysys->GetExpireDate(&isExpireType, &expYear, &expMonth, &expDay, &expHour, &expMinute);
	//if (isExpireType)
	//{
	//	boost::gregorian::date exp(expYear, expMonth, expDay);
	//	boost::gregorian::date now(boost::gregorian::day_clock::local_day());

	//	boost::gregorian::date_duration left = exp - now;	
	//	days = left.days();
	//}
	//else days = -1;
	////executions remaining
	//entrysys->GetExecutionsRemaining(&isExpireType, &execs_left);
	//execs = (isExpireType) ? execs_left : -1;
}
//
// Drawables Constructor
//
Tools::Tools()
{
	int count = s_plugins.loadPlugins("tools","tool");
	initializeTools();
}
//
// destruct and unload
//
Tools::~Tools()
{
	/*unload plugins*/ 
	s_plugins.unloadPlugins(); 
}
void Tools::displayMessages()
{
	uint i=0;
	static char expire_mess[128];

	/*do license checking first*/ 
	//for (;i<s_plugins.size(); i++)
	//{
	//	Tool *tool = s_plugins[i]->getTool();

	//	Tool::VersionInfo *vi = (Tool::VersionInfo*)tool->versionInfo();
	//	
	//	if (vi->execs_left > 0) 
	//	{
	//		/* show message */ 
	//		sprintf(expire_mess, _CXR(strUsesMessage).c_str(), vi->execs_left);
	//		ptui::messageWin(300, 80, "images\\3d_stereoscopic.tif", expire_mess, 3);
	//	}
	//	else if (vi->execs_left == 0)
	//	{
	//		/* show message */ 
	//		sprintf(expire_mess, _CXR(strTrialFinished).c_str());
	//		ptui::messageWin(300, 80, "images\\3d_stereoscopic.tif", expire_mess, 3);
	//	}
	//}
	/*displays licensing messages*/ 
}
void Tools::initializeTools()
{
	uint i=0;
	/*do license checking first*/ 
	for (;i<s_plugins.size(); i++)
	{
		Tool *tool = s_plugins[i]->getTool();

		Tool::VersionInfo *vi = (Tool::VersionInfo*)tool->versionInfo();
		if (!vi) exit(0);

		/*check if this needs to be licensed*/ 
		unsigned short **__ls = (unsigned short **)vi->license_status;
		unsigned short *_ls = *__ls;
		unsigned short &ls = *_ls;

		//if (ls == Tool::LicUndetermined)
		//{
		//	/*check the license status*/ 
		//	int lic_code = checkLicense(vi->plugin, vi->localpwd, vi->globalpwd);
		//	ls = (unsigned short)handleLicense(lic_code);

		//	if (ls == Tool::LicCorrupt)
		//	{
		//		vi->execs_left = 0;
		//		vi->days_left =-1;
		//	}
		//	else getExpiryInfo(vi->execs_left, vi->days_left);
		//}
		(*vi->check) &= ~Tool::CheckUnchecked;
	}
	EventInfo e;
	e.event = CP_STARTUP_EVENT;
	e.event_string = "Initializing Tool Plugins:";

	CommandPipe::RequestObj r;
	Tool::requestModule(&r);
	r.module()->event(e);

	/*initialize tools */ 
	for (i=0; i<s_plugins.size(); i++)
	{	
		e.event_string = s_plugins[i]->identifier;
		r.module()->event(e);

		s_plugins[i]->getTool()->initialize();
	}
	//CoUninitialize();

	Tool::registerCmd("openPluginsManager", &Tools::openPluginsManager);
	Tool::registerCmd(_CXR(strLicensePlugin).c_str(), &Tools::unlockPlugin);
}
//
// license state string
//
bool Tools::formatPluginString(Tool *t, char *buffer)
{
	Tool::VersionInfo *vi = (Tool::VersionInfo*)t->versionInfo();
	unsigned short **__ls = (unsigned short **)vi->license_status;
	unsigned short *_ls = *__ls;
	unsigned short &ls = *_ls;

	if (ls == Tool::LicCorrupt)
	{
		strcpy(buffer, _CXR(strCorrupt).c_str());
		return false;
	}
	else if (vi->execs_left > 0)
	{
		sprintf(buffer, _CXR(strExecLimit).c_str(), vi->execs_left);
	}
	else if (vi->days_left > 0)
	{
		sprintf(buffer, _CXR(strDaysLimit).c_str(), vi->days_left);
	}
	else if (vi->days_left == -1 && vi->execs_left == -1)
	{
		strcpy(buffer, _CXR(strLicensed).c_str());
		return true;
	}	
	else if (!vi->execs_left || !vi->days_left)
	{
		strcpy(buffer, _CXR(strExpired).c_str());
	}
	return false;
}
//
// tools plugin manager
//
void Tools::openPluginsManager()
{	
	Tool::buildWindow("scripts\\ui\\plugins_manager.script", &_params);
	_pluginsPanel = (ptui::Panel*)Tool::panelPointer("pluginsLic.main");
	
	//alwaysAssertM(_pluginsPanel, "Panel pointer retreival failure");
	_pluginsGrid = static_cast<ptui::Grid*>(_pluginsPanel->getGrid("plugins"));
	
	//alwaysAssertM(_pluginsGrid, "Grid pointer retreival failure");

	_pluginsGrid->resize(_pluginsGrid->x(), _pluginsGrid->y(), 560, _pluginsGrid->h());
	_pluginsGrid->columnWidth(0, 110);
	_pluginsGrid->columnWidth(1, 95);
	//_pluginsGrid->columnWidth(2, 260);
	_pluginsGrid->header()->insertItem(0, "Plugin");
	_pluginsGrid->header()->insertItem(1, _CXR(strLicense).c_str());
	_pluginsGrid->header()->insertItem(2, "Description");
	_pluginsGrid->selectMode(ptui::GridSelectRow);
	_pluginsGrid->selectCallback(&Tools::selectPlugin);

	char lic[64];

	/*initialize tools */ 
	for (int i=0; i<s_plugins.size(); i++)
	{
		Tool *tool = s_plugins[i]->getTool();

		_pluginsGrid->insertItem(0,i, s_plugins[i]->identifier);
		_pluginsGrid->insertItem(2,i, s_plugins[i]->getDescription());

		if (!formatPluginString(tool, lic))
		{
			_pluginsGrid->insertItem(1, i, lic);
			_pluginsGrid->cellForeColor(0, i, ptui::Preferences::color(ptui::Hilight_Color));
			_pluginsGrid->cellForeColor(2, i, ptui::Preferences::color(ptui::Hilight_Color));
			_pluginsGrid->cellForeColor(1, i, ptui::Preferences::color(ptui::Hilight_Color));
		}
		else
		{
			_pluginsGrid->insertItem(1, i, lic);
		}
	}
	Tool::showWindow("pluginsLic");
}
void Tools::selectPlugin()
{
	if (_pluginsGrid->numberSelectedCells())
	{
		ptui::Cell *cell = _pluginsGrid->selectedCell(0);

		char lic[64];

		if (cell)
		{
			_current = s_plugins[cell->y()]->getTool();
			_params.set("can_unlock", !formatPluginString(_current, lic));
			_params.set("plugin_name", pt::String(s_plugins[cell->y()]->identifier));
			_params.set("plugin_desc", pt::String(s_plugins[cell->y()]->getDescription()));
			_params.set("plugin_lic", pt::String(lic));
			
			Tool::updateUI("pluginsLic.main");
		}
	}
}
void Tools::unlockPlugin()
{
	if (!_current) return;

	//Tool::VersionInfo *vi = (Tool::VersionInfo*)_current->versionInfo();
	//unsigned short **__ls = (unsigned short **)vi->license_status;
	//unsigned short *_ls = *__ls;
	//unsigned short &ls = *_ls;

	//long return_code = 1000;
	//
	//switch(ls)
	//{
	//case Tool::LicCorrupt:
	//	/* recover */ 
	//	entrysys->RequestActivationDialog(0, &return_code);
	//	break;
	//case Tool::LicEvaluation:
	//case Tool::LicExpired:
	//	entrysys->RequestActivationDialog(2, &return_code);
	//	break;
	//}
	//if (return_code < 500)
	//{
	//	/* tell user to restart*/ 
	//	Tool::okMessage(_CXR(strPluginLicensing).c_str(), _CXR(strActivateSuccess).c_str());
	//	
	//	/*show updated field */ 
	//	ptui::Cell *cell = _pluginsGrid->selectedCell(1);
	//	cell->text(_CXR(strActivated).c_str());
	//	cell->removeColor();
	//	cell = _pluginsGrid->selectedCell(0);
	//	cell->removeColor();
	//}
	//else if (return_code != LIC_FALSE_REFUSE_ACTIVATE)
	//{
	//	/* tell user error*/ 
	//	Tool::errorMessage(_CXR(strPluginLicensing).c_str(), _CXR(strCodeInvalid).c_str());
	//}
}	


