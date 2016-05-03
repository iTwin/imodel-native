/*--------------------------------------------------------------------------*/ 
/*	Pointools Modules manager class definition								*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 21 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifdef _WIN32
#pragma warning (disable : 4786)
#pragma warning (disable : 4251)
#endif

#include <ptmodm/modulesManager.h>
#include <pt/trace.h>

using namespace pt;
 
//
ModulesManager::ModulesManager()
{
//	PTTRACE("ModulesManager::ModulesManager");
}
ModulesManager::~ModulesManager()
{
//	PTTRACE("ModulesManager::~ModulesManager");
}
void ModulesManager::clearModules()
{
	m_modules.clear();
}
//
bool ModulesManager::requestModule(pt::RequestObj *r)
{
	bool ret = false;

	for (unsigned int i=0; i<m_modules.size(); i++)
	{
		if ((*m_modules[i]) == (*r))
		{
			r->_module = m_modules[i];
			ret = true;
			break;
		}
	}

	return ret;
}
//
bool ModulesManager::addModule(_Module *m)
{
//	PTTRACE("ModulesManager::addModule");
	if (!m) 
	{
//		PTTRACEOUT << "null module passed";
		return false;
	}
	/*check for version/id conflicts*/ 
	if (!hasModule(m))
	{
		m_modules.push_back(m);
		return true;
	}
//	PTTRACEOUT << "MODULESMANAGER: WARNING: module unable to load due to identifier conflict";
	return false;
}
//
bool ModulesManager::hasModule(_Module *m)
{
//	PTTRACE("ModulesManager::hasModule");

	for (unsigned int i=0; i<m_modules.size(); i++)
		if (m_modules[i]->moduleID() == m->moduleID() 
			&& m_modules[i]->versionID() == m->versionID())
			return true;
	return false;
}
