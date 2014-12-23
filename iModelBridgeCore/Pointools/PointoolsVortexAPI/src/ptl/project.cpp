/*--------------------------------------------------------------------------*/ 
/*	Pointools Project														*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 29 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#include <ptl/project.h>
#include <ptl/block.h>
#include <ptappdll/ptapp.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ptl/dispatcher.h>
#include <ptl/projecttool.h>

#include <boost/bind.hpp>
#include <pt/trace.h>

#include <set>

using namespace ptl;
using namespace ptds; 

#define POINTOOLS_1_53_TAG 20042005

//
// Globals	force visibility accross process/heaps ect
//			found statics not reliable enough
//
namespace __ptl
{
#ifndef POINTOOLS_API_INCLUDE
	pt::Project pjtool;
#endif
	extern std::vector<clear_cb> clearhandlers;
	extern std::vector<clear_cb> openhandlers;
	Handler *_handlers[3];
	Project *_project;
	BranchHandler *branchHandler[3];

	void clear()
	{
		Project::project()->clearConfigs();

		for (unsigned int i=0; i<clearhandlers.size(); i++)
			clearhandlers[i]();
	}
	void notifyOpen()
	{
		for (unsigned int i=0; i<openhandlers.size(); i++)
			openhandlers[i]();		
	}
}
void Project::clearConfigs()
{
	PTTRACE("Project::clearConfigs");

	Configurations::iterator it = _configs.begin();
	Configuration *cg = _configs.find("Start-up")->second;
	
	while (it != Project::project()->_configs.end())
	{
		if (it->second != cg) delete it->second;
		++it;
	}
	_configs.clear();
	_configs.insert(Configurations::value_type("Start-up", cg));
}
//
// Constructor
//
Project *Project::project() 
{
	if (!__ptl::_project)
		__ptl::_project = new Project;
	return __ptl::_project;
}
Project::Project()
{
	/*setup default values*/ 
	_block_count =0;
	_blocks_read = 0;
	_modified = false;

	__ptl::_project = this;

	static bool reg = false;
	if (!reg || __ptl::_handlers[0] == 0)
	{
		/*register block handlers for 1.56*/ 
		__ptl::_handlers[0] = new Handler("PROJPROP", &Project::PROJPROPR, &Project::PROJPROPW);
		__ptl::_handlers[1] = new Handler("VERSION_", &Project::VERSION_R, &Project::VERSION_W);
		//__ptl::_handlers[2] = new Handler("CONFIGBK", &Project::CONFIG_R, &Project::CONFIG_W);

		/*register branch handlers */ 
		__ptl::branchHandler[0] = new BranchHandler("Project", &Project::ReadProjectBranch, &Project::WriteProjectBranch);
		__ptl::branchHandler[1] = new BranchHandler("Product", &Project::ReadVersionBranch, &Project::WriteVersionBranch);

		__ptl::branchHandler[2] = new BranchHandler("Configurations", &Project::readConfigsBranch, &Project::writeConfigsBranch);
	}
}
void Project::initialize()
{
#ifndef POINTOOLS_API_INCLUDE
	__ptl::pjtool.initialize();
#endif
}
Project::~Project() 
{
	delete __ptl::_handlers[0];
	delete __ptl::_handlers[1];
	delete __ptl::_handlers[2];
	delete __ptl::branchHandler[0];
	delete __ptl::branchHandler[1];
	delete __ptl::branchHandler[2];
}
//
//access
//
const char* Project::title() const { return _title.c_str(); }
const char* Project::author() const { return _author.c_str(); }
const char* Project::company() const { return _company.c_str(); }
const char* Project::keywords() const { return _keywords.c_str(); }
const char* Project::comments() const { return _comments.c_str(); }
//
//modification
//
void Project::title(const char *s) { if (s) _title = s; else _title = ""; }
void Project::author(const char *s) { if (s) _author = s; else _author = ""; }
void Project::company(const char *s) {  if (s) _company= s; else _company = ""; }
void Project::keywords(const char *s) {  if (s) _keywords = s; else _keywords = "";}
void Project::comments(const char *s) {  if (s) _comments = s; else _comments = ""; }

bool Project::WriteProjectBranch(pt::datatree::Branch *br)
{
	PTTRACE("Project::WriteProjectBranch");

	br->addNode("Title", pt::String(project()->title()));
	br->addNode("Author", pt::String(project()->author()));
	br->addNode("Company", pt::String(project()->company()));
	br->addNode("Keywords", pt::String(project()->keywords()));
	br->addNode("Comments", pt::String(project()->comments()));
	return true;
}
bool Project::ReadProjectBranch(const pt::datatree::Branch *br)
{
	PTTRACE("Project::ReadProjectBranch");
	
	pt::String s;
	br->getNode("Title", s);
	project()->title(s);

	br->getNode("Author", s);
	project()->author(s);

	br->getNode("Company", s);
	project()->company(s);

	br->getNode("Keywords", s);
	project()->keywords(s);

	br->getNode("Comments", s);
	project()->comments(s);

	return true;
}
bool Project::WriteVersionBranch(pt::datatree::Branch *br)
{
	br->addNode("Product", pt::String("Pointools View"));
	br->addNode("Version0", 1);
	br->addNode("Version1", 6);
	br->addNode("Version2", 0);
	br->addNode("Version3", 0);
	return true;
}
bool Project::ReadVersionBranch(const pt::datatree::Branch *br)
{
	/*no need to read anything*/ 
	return true;
}
//
//filepath
//
static char fn[PT_MAXPATH];

void Project::setFilepath(const ptds::FilePath &fp)
{
	/*no validation at this point*/ 
	_filepath = fp;
	ptds::FilePath::setProjectDirectory(fp.path());
	fn[0] = '.';
}
const ptds::FilePath* Project::filepath() const { return &_filepath; }

const char* Project::filename() const 
{
	if (fn[0] == '<') return fn;

	if (filepath()->isEmpty())
		strcpy(fn, modified() ? "<unnamed>" : " ");
	else
		strncpy(fn, pt::Unicode2Ascii::convert(filepath()->path()).c_str(), 260);
	return fn;
}
//=============================================================================================
// file operations
//=============================================================================================
struct BranchDispatcher
{
	void operator()(pt::datatree::Branch *br)
	{
		if (!Dispatcher::instance()->dispatchBranch(br))
		{
//#ifdef DATATREE_DEBUGGING	
	char brid[NODE_ID_SIZE];
	br->id().get(brid);
	std::cout << "unable to find branch handler for " << brid << std::endl;
//#endif
		}
	}
};
bool Project::open()
{
	std::cout << "Project::Open\n";
	PTTRACE("Project::open");

	/* version 1.6 and later */ 
	if (_filepath.checkExists())
	{
		pt::datatree::DataTree dt( pt::String(_filepath.path()) );
		
		dt.setReadOnly();
		BranchDispatcher bd;
		dt.visitBranches(bd);

		_modified = false;
	}
	else _error = "Cannot access location";

	return false;
}
//=============================================================================================
// save operation
//=============================================================================================
bool Project::save()
{
	PTTRACE("Project::save");

	_error = "No Error";

	pt::datatree::DataTree dt;
	Dispatcher::instance()->writeTree(&dt);
	bool result =  dt.writeTree( pt::String(_filepath.path()) );
	
	/*pt::datatree::ListBranchVisitor v;
	dt.visitNodes(v);*/
	dt.clear();

	_modified = false;
	return result;
}
//
// clear data from project
//
void Project::reset()
{
	std::cout << "Project::reset\n";
	PTTRACE("Project::reset");

	_filepath = "";
	_title = "";
	_author = "";
	_company = "";
	_comments = "";
	_keywords = "";
	__ptl::clear();
}
//=============================================================================================
// PROJECT_ block Handlers
//=============================================================================================
bool Project::PROJPROPR(const Block *block)
{
	char buffer[1024];
	block->read_s(buffer);
	project()->title(buffer);

	block->read_s(buffer);
	project()->author(buffer);

	block->read_s(buffer);
	project()->company(buffer);

	block->read_s(buffer);
	project()->keywords(buffer);

	block->read_s(buffer);
	project()->comments(buffer);

	return true;
}
//
// read version block
//
bool Project::VERSION_R(const Block *block)
{
	int version = 1;
	block->read(version);
	return true;
}
//
// write block
//
Block *Project::PROJPROPW()
{	
	Block *block = Block::allocBlock();
	memcpy(block->identifier, "PROJPROP", 8);

	/*version*/ 
	block->write_s(project()->title());
	block->write_s(project()->author());
	block->write_s(project()->company());
	block->write_s(project()->keywords());
	block->write_s(project()->comments());

	return block;
}
//
// write version block
//
Block *Project::VERSION_W()
{
	Block *block = Block::allocBlock();
	memcpy(block->identifier, "VERSION_", 8);

	int version = 1;
	block->write(version);
	return block;
}