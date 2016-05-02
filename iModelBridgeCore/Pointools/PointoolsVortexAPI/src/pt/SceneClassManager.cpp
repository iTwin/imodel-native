#include "PointoolsVortexAPIInternal.h"

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <pt/sceneclassmanager.h>
#include <iostream>

//
// plugins manager
//
#define NUMSYM 1
typedef  bool			(*INITIALIZE)(void);

#define SYM1			"initializePlugin"
#define SYM1MARGS       "v"
#define FUNC1			INITIALIZE
#define FUNC1IDENTIFIER	bool initialize

#include <ptapp\pluginsmanager.h>
static ptapp::PluginsManager s_plugins("getName");

#undef INITIALIZE
#undef NUMSYM

using namespace pt;

int SceneClassManager::loadSceneClassManagers()
{
	PTTRACE("SceneClassManager::loadSceneClassManagers");

	s_plugins.loadPlugins(_T("drawables"),_T("ddb"));
	for (uint i=0; i<s_plugins.size(); i++)
	{
		s_plugins[i]->initialize();
	}
	return s_plugins.size();
}
#endif