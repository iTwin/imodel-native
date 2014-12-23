/*--------------------------------------------------------------------------*/ 
/*	Pointools Block															*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 28 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef PTLDISPATCHER_DEFINITION_HEADER
#define PTLDISPATCHER_DEFINITION_HEADER	1

#include <ptl/block.h>
#include <ptl/branch.h>
#include <ptfs/filepath.h>

namespace ptl
{
//
// Block Dispatcher
//
class PTL_API Dispatcher
{
public:
	Dispatcher();
	~Dispatcher();

	static Dispatcher *instance();

	/*state*/ 
	bool	isWriting() const { return _writing; }

	/*register handler*/ 
	bool	registerHandler(Handler* handler);

	bool	registerBranchHandler(BranchHandler *handler);
	bool	registerConfigurationHandler(BranchHandler* handler);

	void	registerClearHandler(clear_cb cb);
	void	registerOpenHandler(clear_cb cb);

	void	unregisterHandler(const char id[8]);
	void	unregisterBranchHandler(pt::datatree::NodeID id, bool configuration);

	bool	dispatchBlock(const Block *block);
	bool	dispatchBranch(pt::datatree::Branch *br, bool configuration = false);

	int		writeBlocks(Blocks &blocks);
	bool	writeBlock(Block *block, bool configuration = false);

	int		writeTree(pt::datatree::Branch *tree, bool configuration = false);

	void	freeBlocks(Blocks &blocks);
	
	const char* blockHandlerName(const char *id) const;

private:
	bool _writing;
};
struct BlockIO
{
	static Block *	load	(ptds::DataSourcePtr h);
	static bool		store	(Block *block, ptds::DataSourcePtr h);	
};
}
#endif