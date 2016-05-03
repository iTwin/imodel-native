/*--------------------------------------------------------------------------*/ 
/*	Pointools Tools application module class definition						*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 28 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_TOOLS_MODULE_INTERFACE
#define POINTOOLS_TOOLS_MODULE_INTERFACE

#include <pttool/pttool.h>
#include <ptmodm/ModulesManager.h>
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