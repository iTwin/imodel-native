/*--------------------------------------------------------------------------*/ 
/*	Pointools Licensing System												*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 1 March 2004 Faraz Ravi									*/ 
/*--------------------------------------------------------------------------*/ 
#include <ptapp/license.h>
#include <stdio.h>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <ptuim/uimanager.h>
#include <ptapp/application.h>
#include <ptappdll/ptapp.h>
#include <fltk/ask.h>

#include <iostream>
#include <ptfs/filepath.h>
#include <pcguard/pcgi.h>
#include <cxr_inc.h>

#include <fstream>

using namespace ptapp;
bool s_initialized = false;

#define _VERBOSE 1

using namespace boost::gregorian;

extern const char*strClientProt;
extern const char*strDateFormat;
extern const char*strLocalPwd;
extern const char*strGlobalPwd;
extern const char*strPtv15;
extern const char*strLic;
extern const char*strLicLoc;

namespace { Validate plug; }

struct lli
{
	lli() 
	{
		info.PCGI_Size = sizeof(info);
		valid = (GetInterfaceData(&info) == PCGI_STATUS_OK);
	}
	~lli() { ZeroMemory(&info, sizeof(info)); }
	PCG_INTERFACE			info;		
	bool valid;
};

template <class T> struct SetVal { 
template <class A> static void set(void *ptr, const A &val) { *((T*)ptr) = (T)val; }
};
void *ptapp::validateObject()
{
	return &plug;
}
struct CoutTrace { CoutTrace(const char *m) { std::cout << m << "{ " << std::endl; } ~CoutTrace() { std::cout << "}" << std::endl; } 
	template <class A> void output(A a) {  std::cout << a << std::endl; }
};
#define PTTRACE(a) CoutTrace bob(a)
#define PTOUT(a) bob.output(a)

//-------------------------------------------------------------------------
// check the license is valid
//-------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0001(void *ret) //inEvaluationMode()
{
	lli li;
	if (!li.valid)  return 0; 

	SetVal<bool>::set(ret, (bool)li.info.PCGI_DemoModeActive);
	return 1;
}
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0002(void *ret)//inLimitedMode()
{
	lli li;
	if (!li.valid)  return 0; 

	SetVal<bool>::set(ret, li.info.PCGI_LimitedLicenseActive);
	return 1;
}
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0003(void *ret)//evaluationDaysRemaining()
{
	lli li;
	if (!li.valid)  return 0; 

	SetVal<int>::set(ret, li.info.PCGI_DemoDaysLeft);
	return 1;
}
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0004(void *ret)//limitedTimeRemaining()
{
	lli li;
	if (!li.valid)  return 0; 

	SetVal<int>::set(ret, li.info.PCGI_LimitedLicenseDaysLeft);
	return 1;
}
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0005(void *sitecode, void *machineid)//siteInfo(void *sitecode, void *machineid)
{
	lli li;
	if (!li.valid) return 0; 

	memcpy(sitecode, &li.info.PCGI_SiteCode, sizeof(li.info.PCGI_SiteCode));
	memcpy(machineid, &li.info.PCGI_MachineID, sizeof(li.info.PCGI_MachineID));
	return true;
}
//--------------------------------------------------------------------------------
// Unlock application
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0006(void *code)//activatePermenantLicense(void *code)
{
	lli li;
	if (!li.valid) return 0;

	return (UnlockApplication((char*)code) == PCGI_SUCCESS_APPLICATION_UNLOCKED) ? 1 : 0;
}
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0007(void *code)//extendEvaluation(void *code)
{
	lli li;
	if (!li.valid) return 0;

	return (UnlockApplication((char*)code) == PCGI_SUCCESS_DEMO_EXTENDED) ? 1 : 0;
}
//--------------------------------------------------------------------------------
// Network licensing
//--------------------------------------------------------------------------------
//bool /*License*/Validate::activateNetworkLicense(code, sites)
//{
//	lli li;
//
//}

//--------------------------------------------------------------------------------
// Edit licensing
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0008(void *sitecode, void *newcode)//transferLicense(void *sitecode, void *newcode)
{
	lli li;
	if (!li.valid) return 0;

	return (TransferLicense(*((DWORD*)sitecode), (char*)newcode) == PCGI_STATUS_OK) ? 1 : 0;
}
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0009(void *removalcode)//removeLicense(void *removalcode)
{
	lli li;
	if (!li.valid) return 0;

	return (RemoveLicense((LPDWORD)removalcode) == PCGI_STATUS_OK) ? 1 : 0;
}
//--------------------------------------------------------------------------------
PTLIC_RESULT /*License*/Validate::_0010()//freeAppInstance
{
	lli li;
	if (!li.valid) return 0;

	return (FreeAppInstance() == PCGI_STATUS_OK) ? 1 : 0;
}
