/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/ 
/*	Pointools Tools application module class definition						*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_TOOLS_MODULE_INTERFACE
#define POINTOOLS_TOOLS_MODULE_INTERFACE

#include <pttool/pttool.h>
#include <ptmodm/modulesManager.h>
#include <ptmodm/module.h>

namespace pt { class Tool; }
namespace ptapp
{
	class PTTOOL_API Tools : public pt::Module<Tools, 1900, 1000>
	{
	public:
		Tools();
		~Tools();
		
		static void openPluginsManager();
		static void selectPlugin();
		static void unlockPlugin();
		
		static void displayMessages();

	private:
		void initializeTools();
		static bool formatPluginString(pt::Tool *t, char *buffer);
	};
}
#endif
