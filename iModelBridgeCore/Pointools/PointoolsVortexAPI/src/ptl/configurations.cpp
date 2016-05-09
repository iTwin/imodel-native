/*--------------------------------------------------------------------------*/ 
/*	Pointools Configurations												*/ 
/*  (C) 2005 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 02 May 2005 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#include "PointoolsVortexAPIInternal.h"
#include <ptl/project.h>
#include <ptl/block.h>
#include <ptappdll/ptapp.h>



#include <ptl/dispatcher.h>
#include <ptl/projecttool.h>

#include <pt/datatreePrintf.h>


#include <set>
#include <iostream>

using namespace ptl;
using namespace ptds; 

#define PT_MAX_CONFIG_COMPONENTS 10

//
// Configuration constructor
//
Project::Configuration::Configuration(const char *id, bool empty) : tree(id)
{
	if (id)	strncpy(identifier, id, 64);
	if (!empty) 
	{
		Dispatcher::instance()->writeTree(&tree, true);	
	}
}
Project::Configuration::~Configuration()
{
	tree.clear();
}
//
// read configurations branch
//
struct CopySubbranchVisitor : private Project::Configuration
{
	char id[64];

	void operator()(const pt::datatree::Branch *br)
	{
		br->id().get(id);

		Project::Configuration *config = Project::project()->createConfiguration(id, true);		
		(*config->configs())= *br;
	}
};

bool Project::readConfigsBranch(const pt::datatree::Branch *branch)
{
	return true;

	/*copy subbranches*/ 
	CopySubbranchVisitor v;
	branch->visitBranches(v);

	return true;
}
bool Project::writeConfigsBranch(pt::datatree::Branch *branch)
{
	return true;

	Configurations::iterator it = Project::project()->_configs.begin();

	/*don't save the startup configuration*/ 
	while (it != Project::project()->_configs.end())
	{
		if (strcmp("Start-up", it->first.c_str())!=0)
			branch->addBranchCopy(it->second->configs());	
		++it;
	}
}
//
/*configuration handling																			*/ 
//
struct ConfigurationBuilder : public Project::Configuration
{
	ConfigurationBuilder(const char *id, bool empty=false) : Project::Configuration(id, empty) {};
};
Project::Configuration *Project::createConfiguration(const char *identifier, bool empty)
{
	Project::Configuration *config = new ConfigurationBuilder(identifier, empty);
	bool success = _configs.insert(Configurations::value_type(identifier, config)).second;
	if (success && _configs.size() > 1) project()->modify();
	
	if (!success) 
	{
		delete config;
		config = 0;
	}
	return success ? config : 0;
}
//
// Remove a configuration
//
bool Project::removeConfiguration(const char *identifier)
{
	Configurations::iterator it = _configs.find(identifier);
	if (it != _configs.end()) 
	{
		delete it->second;
		_configs.erase(it);
		_modified = true;
		return true;
	}
	return false;
}
//
//
//
Project::Configuration *Project::getConfiguration(const char *id)
{
	Configurations::iterator it = _configs.find(id);
	return (it == _configs.end()) ? 0 : it->second;
}
//
//
//
int Project::numConfigurations() const { return static_cast<int>(_configs.size()); }
//
//
//
int Project::getConfigurationNames(std::string *ids) const
{
	int i = 0;
	Configurations::const_iterator it = _configs.begin();
	while (it != _configs.end())
	{
		ids[i++] = it->first;
		++it;
	}
	return i;
}
//
// Configuration::Apply
//
struct ComponentApplyVisitor
{
	ComponentApplyVisitor() : count(0) {};
	void operator()(pt::datatree::Branch *br)
	{
		if (br->flags(0)==0)
			Dispatcher::instance()->dispatchBranch(br, true);
	}
	int count;
};
void Project::Configuration::apply()
{
#ifdef _DEBUG
	pt::datatree::ListBranchVisitor pn;
	tree.visitNodes(pn);
#endif

	/*iterate branches and dispatch*/ 
	ComponentApplyVisitor v;
	tree.visitBranches(v);
}
void  Project::Configuration::component(int i, const char **desc, bool &enabled)
{
	static pt::String strings[PT_MAX_CONFIG_COMPONENTS];

	pt::datatree::Branch * br = tree.getNthBranch(i);
	
	if (!br)
	{
		(*desc) = 0;
		return;
	}
	char id[64];
	br->id().get(id);
	strings[i] = id;
	(*desc) = strings[i].c_str();

	enabled = !(br->flags(0));
}
void Project::Configuration::enableComponent(int i, bool enable)
{
	pt::datatree::Branch * br = tree.getNthBranch(i);
	if (br)
	{
		br->setFlags(0, enable ? 0 : 1);
	}
	project()->modify();
}
void Project::Configuration::applyComponent(int i)
{
	pt::datatree::Branch * br = tree.getNthBranch(i);
	if (br) Dispatcher::instance()->dispatchBranch(br, true);
}
