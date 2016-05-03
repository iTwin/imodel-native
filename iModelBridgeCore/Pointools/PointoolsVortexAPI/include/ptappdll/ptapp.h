/*--------------------------------------------------------------------------*/ 
/*	Pointools App class class												*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 30 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#include <ptmodm/ModulesManager.h>

namespace pt {class ParameterMap; }

namespace ptapp
{
	pt::ModulesManager *application();

	const char* applicationString();
	const char* versionString();
	const char* fullVersionString();
	const char* editionString();

	void _setapp(pt::ModulesManager *m);
	const wchar_t* apppathW();
	const char* apppathA();

	void setAppPathA(const char *path);
	void setAppPathW(const wchar_t *path);

	const wchar_t* apppath();
	void setAppPath(const wchar_t *path);

	void absolutePath(char *path, const char* rel);

	pt::ParameterMap *env();
}