/*--------------------------------------------------------------------------*/ 
/*	Pointools App class class												*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 30 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include "PointoolsVortexAPIInternal.h"
#include <ptappdll/ptapp.h>
#include <pt/parametermap.h>
#include <ptfs/filepath.H>

using namespace pt;
using namespace ptapp;

#define PTVIEW_UNKNOWN_VERSION 0
#define PTVIEW_STD_VERSION 1
#define PTVIEW_PRO_VERSION 2
#define PTVIEW_CYCLOPS_VERSION 3

namespace 
{
	ModulesManager *__app = 0;
	ParameterMap _environment; 
	ptds::FilePath _apppath;
	std::string _apppathA;
	bool _pathinit = false;
	int _version = PTVIEW_UNKNOWN_VERSION;
	char _appString[64];
}
ModulesManager *ptapp::application()
{
	return __app;
}
void ptapp::_setapp(ModulesManager *m)
{
	__app = m;
}
static void determineVersion()
{
	static bool init = false;
	if (!init)
	{
		char path[MAX_PATH];
		::GetModuleFileNameA(NULL, path, sizeof(path));

		if (strcmp(&path[strlen(path)-13],"PTViewPro.exe")==0)
		{
			strcpy(_appString, "Pointools View 1.6 Pro");
			_version = PTVIEW_PRO_VERSION;
		}
		else if (strcmp(&path[strlen(path)-11],"Cyclops.exe")==0)
		{
			strcpy(_appString, "Cyclops 1.1");
			_version = PTVIEW_CYCLOPS_VERSION;
		}
		else if (strcmp(&path[strlen(path)-10],"PTView.exe")==0)
		{
			strcpy(_appString, "Pointools View 1.6");
			_version = PTVIEW_STD_VERSION;
		}
		else
		{
			strcpy(_appString, "Pointools View");
			_version = PTVIEW_UNKNOWN_VERSION;
		}
		init = true;
	}
}
//
// application string
//
const char *ptapp::applicationString()
{
	determineVersion();
	return _appString;
}
//
// version string
//
const char *ptapp::versionString()
{
	determineVersion();
	return (_version == PTVIEW_CYCLOPS_VERSION) ? "1.1" : "1.6";
}
//
// version string
//
const char *ptapp::fullVersionString()
{
	return "1.6061113";  
}
//
// edition string
//
const char *ptapp::editionString()
{
	determineVersion();
	return (_version == PTVIEW_PRO_VERSION) ? "Pro" : "Std";
}
//
// application path
//
const wchar_t*ptapp::apppathW()
{
	if (!_pathinit)
	{
		wchar_t fp[PT_MAXPATH];

		GetModuleFileNameW(NULL, fp, PT_MAXPATH);
		_apppath.setPath(fp);
		_apppath.stripFilename();

		_pathinit = true;
		_apppathA = Unicode2Ascii::convert(_apppath.path());
	}
	return _apppath.path();
}
const char*ptapp::apppathA()
{
	if (!_pathinit) apppathW();
	return _apppathA.c_str();
}
pt::ParameterMap *ptapp::env() { return &_environment; } 
//
// set application path - used by plugins to other hosts
//
void ptapp::setAppPathA(const char *path)
{
	_apppath = ptds::FilePath(path);
	_pathinit = true;
	_apppathA = std::string(path);	
}
void ptapp::setAppPathW(const wchar_t *path)
{
	_apppath = ptds::FilePath(path);
	_pathinit = true;
	_apppathA = Unicode2Ascii::convert(_apppath.path());	
}
const wchar_t* ptapp::apppath() { return apppathW(); }
void ptapp::setAppPath(const wchar_t *path) { setAppPathW(path); }
