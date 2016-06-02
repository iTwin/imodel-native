/*--------------------------------------------------------------------------*/ 
/*	Modules Manager template base class interface							*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 21 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_MODULES_MANAGER
#define POINTOOLS_MODULES_MANAGER

#include <vector>
#include <string>

#include "module.h"

#ifdef MODM_EXPORTS
#define MODM_API EXPORT_ATTRIBUTE
#else
#define MODM_API IMPORT_ATTRIBUTE
#endif

namespace pt
{

#ifdef BENTLEY_WIN32
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

class MODM_API ModulesManager
{
public:
	ModulesManager();
	virtual ~ModulesManager();

	bool requestModule(RequestObj *r);
	bool hasModule(_Module *m);

protected:
	bool addModule(_Module *m);
	/*this will not delete the modules*/ 
	void clearModules();

	_Module *getModule(unsigned int index) { return m_modules[index]; }
	inline unsigned int numModules() const { return static_cast<int>(m_modules.size()); }

private:
	std::vector<_Module*>	m_modules;
};

#ifdef BENTLEY_WIN32
#pragma warning( pop ) 
#endif
}
#endif