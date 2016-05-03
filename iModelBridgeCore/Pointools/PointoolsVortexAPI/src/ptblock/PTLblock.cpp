#include "include/ptlblock.h"

//PTBLOCK Handler Map
bool ptlblock_dispatcher::register_handler(const char *h, handler_callback cb)
{
	return map().insert(HANDLERMAP::value_type(h, cb)).second;
}
//
// read block
// returns blocks read
//
int	ptlblock_dispatcher::read_block(const ptlblock &block)
{
	HANDLERMAP::iterator i = map().find(block.handler_tag);
	if (i == map().end()) return 0;

	/*dispatch*/ 
	ptlblocks blocks;
	blocks.push_back(block);

	bool v = (i->second)(false, blocks);
	if (block.del_after_use) delete [] block.data;
	return v;
}
//
// write (all handled) blocks
// returns number of blocks written
//
int	ptlblock_dispatcher::write_blocks(ptlblocks &blocks)
{
	HANDLERMAP::iterator i = map().begin();

	int block_count = 0;

	while( i!=map().end())
	{
		block_count += (i->second)(true, blocks);
		i++;
	}
	return block_count;
}
//
// write single block
// returns blocks written
//
int  ptlblock_dispatcher::write_block(ptlblock &block)
{
	HANDLERMAP::iterator i = map().find(block.handler_tag);
	if (i == map().end()) return 0;

	/*dispatch*/ 
	ptlblocks blocks;
	blocks.push_back(block);

	return (i->second)(true, blocks);
}
//
// free memory used by blocks
//
void ptlblock_dispatcher::free_blocks(ptlblocks &blocks)
{
	for (int i=0; i<blocks.size(); i++)
		if (blocks[i].del_after_use)
			delete [] blocks[i].data;
}