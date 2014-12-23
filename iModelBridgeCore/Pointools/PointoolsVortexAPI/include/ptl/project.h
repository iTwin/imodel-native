/*--------------------------------------------------------------------------*/ 
/*	Pointools Project														*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 29 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef PTLPROJECT_DEFINITION_HEADER
#define PTLPROJECT_DEFINITION_HEADER	1

#include <string>
#include <map>

#include <ptl/block.h>
#include <ptl/branch.h>
#include <ptfs/filepath.h>

namespace ptl
{
//
// Pointools Project
//
class PTL_API Project
{
public:	
	Project();
	~Project();

	const char *title() const;
	void title(const char *s);

	const char *author() const;
	void author(const char*s);

	const char *company() const;
	void company(const char*s);

	const char *keywords() const;
	void keywords(const char*s);

	const char *comments() const;
	void comments(const char*s);
	
	/*version of Pointools*/ 
	int		fileCreator();
	int		fileCreatorString();

	int blocks();
	int blocksRead();

	const ptds::FilePath *filepath() const;
	void setFilepath(const ptds::FilePath& fp);

	const char* filename() const;

	bool open();
	bool save();
	bool saveAs();
	bool saveCopy();
	void clear();
	void reset();

	inline bool modified() const { return _modified; }
	inline void modify() { _modified = true; }

	const char* getLastError() const;

	static Project *project();

	void initialize();

	class PTL_API Configuration
	{
	public:
		~Configuration();
		
		void apply();

		int numComponents() const { return tree.numBranches(); }
		void component(int i, const char **desc, bool &enabled);
		
		void enableComponent(int i, bool enable = true);
		void applyComponent(int i); 

		const char* name() const { return identifier; }

		void copyBranch(const pt::datatree::Branch *br);

		pt::datatree::Branch *configs() { return &tree; }
	protected:
		Configuration(const char*id, bool empty);
		Configuration() : tree("null") {};

		char identifier[64];

		pt::datatree::Branch	tree;
	};

	/*configuration handling						*/ 
	Configuration* createConfiguration(const char *identifier, bool empty = false);
	bool updateConfiguration(const char *identifier);
	bool removeConfiguration(const char *identifier);

	int numConfigurations() const;
	Configuration *getConfiguration(const char *id);
	int getConfigurationNames(std::string *ids) const;

	void clearConfigs();

private:
	static bool PROJPROPR(const Block *block);
	static bool VERSION_R(const Block *block);
	static Block *PROJPROPW();
	static Block *VERSION_W();


	static bool readConfigsBranch(const pt::datatree::Branch *branch);
	static bool writeConfigsBranch(pt::datatree::Branch *branch);

	static bool ReadProjectBranch(const pt::datatree::Branch *br);
	static bool WriteProjectBranch(pt::datatree::Branch *br);

	static bool ReadVersionBranch(const pt::datatree::Branch *br);
	static bool WriteVersionBranch(pt::datatree::Branch *br);

	static bool ReadConfigBranch(const pt::datatree::Branch *br);
	static bool WriteConfigBranch(pt::datatree::Branch *br);

	static bool CONFIG_R(const Block *block);
	static Block *CONFIG_W();
	
	std::string		_title;
	std::string		_author;
	std::string		_company;
	std::string		_comments;
	std::string		_keywords;
	std::string		_error;

	typedef std::map<std::string, Configuration*> Configurations;
	Configurations	_configs;

	ptds::FilePath	_filepath;
	int				_block_count;
	int				_blocks_read;

	bool			_modified;

};
}
#endif
