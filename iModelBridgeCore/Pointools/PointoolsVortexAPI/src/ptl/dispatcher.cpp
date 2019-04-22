/*--------------------------------------------------------------------------*/ 
/*	Pointools Dispatcher													*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 28 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include "PointoolsVortexAPIInternal.h"
#include <ptl/block.h>
#include <ptl/dispatcher.h>
#include <pt/trace.h>

using namespace ptl;
using namespace pt;

namespace __ptl
{
	typedef Loki::AssocVector<datatree::NodeID, BranchHandler*> BRANCH_HANDLER_MAP;

	/*handlers used for backward compatibility with View 1.56 and earlier*/ 
	typedef Loki::AssocVector<const char *, Handler*, id_cmp> HANDLERMAP;
	typedef Loki::AssocVector<const char *, const char*, id_cmp> IDMAP;

//	IDMAP		idmap;
	HANDLERMAP	hmap;

	BRANCH_HANDLER_MAP	brmap;
	BRANCH_HANDLER_MAP	cbmap;

	std::vector<clear_cb> clearhandlers;
	std::vector<clear_cb> openhandlers;
}
using namespace __ptl;
using namespace ptds;

//---------------------------------------------------------------------------
// instance uses global to ensure access across dlls
//---------------------------------------------------------------------------
Dispatcher *ptl_dispatcher_instance = 0;
Dispatcher *Dispatcher::instance()
{
	if (!ptl_dispatcher_instance)
	{
		ptl_dispatcher_instance = new Dispatcher;
	}
	return ptl_dispatcher_instance;
}
//---------------------------------------------------------------------------
// Construction / Destruction
//---------------------------------------------------------------------------
Dispatcher::Dispatcher()
{
	_writing = false;
}
Dispatcher::~Dispatcher()
{
	/*cleanup*/ 
}
//---------------------------------------------------------------------------
// registerHandler
//---------------------------------------------------------------------------
bool Dispatcher::registerHandler(Handler *handler)
{
	return hmap.insert(HANDLERMAP::value_type(handler->identifier, handler)).second;
}
//---------------------------------------------------------------------------
// registerHandler
//---------------------------------------------------------------------------
bool Dispatcher::registerBranchHandler(BranchHandler *handler)
{
	return brmap.insert(BRANCH_HANDLER_MAP::value_type(handler->identifier, handler)).second;
}
//---------------------------------------------------------------------------
// register Configuration Handler
//---------------------------------------------------------------------------
bool Dispatcher::registerConfigurationHandler(BranchHandler* handler)
{
	if (cbmap.insert(BRANCH_HANDLER_MAP::value_type(handler->identifier, handler)).second)
	{
//		idmap.insert(IDMAP::value_type(handler->descriptor, handler->identifier));
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
// registerClearHandler
//---------------------------------------------------------------------------
void Dispatcher::registerClearHandler(clear_cb cb)
{
	clearhandlers.push_back(cb);
}
void Dispatcher::registerOpenHandler(clear_cb cb)
{
	openhandlers.push_back(cb);	
}
//---------------------------------------------------------------------------
// unregisterHandler
//---------------------------------------------------------------------------
void Dispatcher::unregisterHandler(const char id[])
{
	HANDLERMAP::iterator it = hmap.find(id);
	if (it != hmap.end())
	{
		hmap.erase(it);
	}
}
//---------------------------------------------------------------------------
// unregisterHandler
//---------------------------------------------------------------------------
void Dispatcher::unregisterBranchHandler(datatree::NodeID id, bool configuration)
{
	BRANCH_HANDLER_MAP::iterator it = configuration ? cbmap.find(id) : brmap.find(id);

	if (configuration && it != cbmap.end())
	{
		cbmap.erase(it);
	}
	else if (it != brmap.end())
	{
		brmap.erase(it);
	}
}
//---------------------------------------------------------------------------
// dispatch a block to its handler - used in retrievel
//---------------------------------------------------------------------------
bool Dispatcher::dispatchBlock(const Block *block)
{
	HANDLERMAP::iterator it = hmap.find(block->identifier);

	if (it != hmap.end())
	{
		it->second->readblock(block);
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
// dispatch a block to its handler - used in retrievel
//---------------------------------------------------------------------------
bool Dispatcher::dispatchBranch(datatree::Branch *br, bool configuration)
{
	BRANCH_HANDLER_MAP::iterator it = configuration ? cbmap.find(br->id()) : brmap.find(br->id());

	if (it != (configuration ? cbmap.end() : brmap.end()))
	{
		it->second->readbranch(br);
		return true;
	}
#ifdef DATATREE_DEBUGGING	
	char brid[32];
	br->id().get(brid);
	std::cout << "unable to find" << (configuration ? " config" : " ") << " branch handler for " << brid << std::endl;
#endif
	return false;
}
//---------------------------------------------------------------------------
// write blocks - used by write methods
//---------------------------------------------------------------------------
int Dispatcher::writeBlocks(Blocks &blocks)
{
	int block_count = 0;
	/*some of these can get quite large for embedded data*/ 
	/*so we need a flush mechanism*/ 
	HANDLERMAP::iterator bg = hmap.begin();
	HANDLERMAP::iterator en = hmap.end();

	HANDLERMAP::iterator it;
	for (it = bg; it != en; it++)
	{
		Block* b = it->second->writeblock();
		if (b)
		{
			blocks.push_back(b);
			/*write to file*/ 
			block_count ++;
		}
	}
	return block_count;
}
//---------------------------------------------------------------------------
// wreite tree
//---------------------------------------------------------------------------
int Dispatcher::writeTree(datatree::Branch *branch, bool configuration)
{
	PTTRACE("Dispatcher::writeTree");

	BRANCH_HANDLER_MAP::iterator it = configuration ? cbmap.begin() : brmap.begin();
	branch->clear();

	while (it != (configuration ? cbmap.end() : brmap.end()))
	{
		datatree::Branch *br = branch->addBranch(it->first);
		it->second->writebranch(br);

		++it;
	}
	return static_cast<int>(brmap.size());
}
//---------------------------------------------------------------------------
// free Blocks allocated here
//---------------------------------------------------------------------------
void Dispatcher::freeBlocks(Blocks &blocks)
{
	for (unsigned int i=0; i<blocks.size(); i++)
	{
		/*data deletetion is responsibility of handler*/ 
		Block::freeBlock(blocks[i]);
	}
	blocks.clear();
}
//---------------------------------------------------------------------------
// free Block
//---------------------------------------------------------------------------
bool Dispatcher::writeBlock(Block *block, bool configuration)
{
	/*find handler*/ 
	HANDLERMAP::iterator it =  hmap.find(block->identifier);
	if (it != hmap.end())
	{
		block = it->second->writeblock();
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
// blockHandlerName
//---------------------------------------------------------------------------
const char*	Dispatcher::blockHandlerName(const char *id) const
{
	HANDLERMAP::const_iterator it = hmap.find(id);
	if (it != hmap.end()) return it->second->descriptor;	
	return 0;
}
//---------------------------------------------------------------------------
// BranchHandler self registering constructor
//---------------------------------------------------------------------------
BranchHandler::BranchHandler(datatree::NodeID id, read_cb rcb, write_cb wcb, bool config)
: writebranch(wcb), readbranch(rcb), identifier(id)
{
	if (!config)
		Dispatcher::instance()->registerBranchHandler(this);
	else 
		Dispatcher::instance()->registerConfigurationHandler(this);

	configuration = config;
}
//---------------------------------------------------------------------------
// Handler self unregistering destructor
//---------------------------------------------------------------------------
BranchHandler::~BranchHandler()
{
	Dispatcher::instance()->unregisterBranchHandler(identifier, configuration);
}
//---------------------------------------------------------------------------
// Handler self registering constructor
//---------------------------------------------------------------------------
Handler::Handler(const char *id, read_cb rcb, write_cb wcb, bool config, const char *desc)
: writeblock(wcb), readblock(rcb)
{
	memcpy(identifier, id, 8);
	if (!config)
		Dispatcher::instance()->registerHandler(this);
//	else 
//		Dispatcher::instance()->registerConfigurationHandler(this);
	
	/*else ignore for now */ 

	if (desc) strncpy(descriptor, desc, sizeof(descriptor));
	else descriptor[0] = '\0';
	configuration = config;
}
//---------------------------------------------------------------------------
// Handler self unregistering destructor
//---------------------------------------------------------------------------
Handler::~Handler()
{
	Dispatcher::instance()->unregisterHandler(identifier);
}
//---------------------------------------------------------------------------
// block allocation + free
//---------------------------------------------------------------------------
Block* Block::allocBlock(void *buffer, size_t sz) 	{ return new Block(buffer, sz); }
Block* Block::allocBlock(size_t sz)					{ return new Block(sz); }
Block* Block::allocBlock()							{ return new Block(); }
void Block::freeBlock(Block *b)						{ delete b; }
//---------------------------------------------------------------------------
//write a sub block allocation + free
//---------------------------------------------------------------------------
void Block::write_block(Block *b)
{ 
	write(b->identifier);
	write(b->version);

	int num_chunks = static_cast<int>(b->firstchunk()->numChunks());
	write(num_chunks);

	Chunk *c = b->firstchunk();

	while (c)
	{
		int size = static_cast<int>(c->size());

		if (size <= 1) size = static_cast<int>(c->totalSize());

		write(size);
		write(c->_chunk, size);
		c = c->next;
	}
}
Block *Block::read_block() const
{	
	Block *b = allocBlock();
	read(b->identifier);
	read(b->version);

	int num_chunks;
	read(num_chunks);

	b->clearChunks();

	for (int i=0; i<num_chunks; i++)
	{
		int size;
		read(size);
		b->addChunk(size);
		read(b->lastchunk()->_chunk, size);
	}
	return b;
}
//---------------------------------------------------------------------------
// load block from file
//---------------------------------------------------------------------------
Block *BlockIO::load(DataSourcePtr h)
{
	Block* block =0 ;
	char id[] = "________";

	if (h->readBytes(id, 8) == 8)
	{
		int	bytes;

		h->readBytes(&bytes, sizeof(int));
		bytes--;

		block = Block::allocBlock(bytes);
		memcpy(block->identifier, id, 8);

		/*sanity check*/ 
		assert(bytes < 5242880);
		if (bytes > 5242880) return 0;

		h->readBytes(block->firstchunk()->_chunk, bytes);
		//printf("\reading %s block %d bytes\n", id, bytes);
	}
	return block;
}
//---------------------------------------------------------------------------
// store block to file
//---------------------------------------------------------------------------
bool BlockIO::store(Block *block, DataSourcePtr h)
{
	if (block)
	{
		int	bytes = static_cast<int>(block->size());
		h->writeBytes(block->identifier, 8);
		h->writeBytes(&bytes, sizeof(int));

		char id[] = "________";
		memcpy(id, block->identifier, 8);

		/*iterate through chunks*/ 
		Chunk *ch = block->firstchunk();
		while (ch)
		{
			h->writeBytes(ch->_chunk, ch->size()-1);
			ch = ch->next;
		}
		//printf("\nwriting %s block %d bytes\n", id, bytes);
		return true;
	}
	return false;
}
