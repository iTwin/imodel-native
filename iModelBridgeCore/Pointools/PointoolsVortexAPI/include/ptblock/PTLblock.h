#ifndef __POINTOOLS_BLOCK_HEADER
#define __POINTOOLS_BLOCK_HEADER

#include <loki/assocvector.h>
#include "../callback/callback.h"
#include "comp.h"

struct ptlblock
{
	unsigned int	type_id;
	char			descriptor[256];
	unsigned int	byte_size;
	char			handler_tag[256];
	unsigned int	block_version;
	bool			del_after_use;
	unsigned char*	data;

	void reset() { byte_size = 0; };

	static int persist_size() { return sizeof(ptlblock) - sizeof(void*) - sizeof(bool); };

	template <class T>
	void write(T s)
	{ memcpy(&data[byte_size], &s, sizeof(T)); byte_size += sizeof(T); };
	
	template <class T> 
	void read(T &s, int &pos) const 
	{ memcpy(&s, &data[pos], sizeof(T)); pos += sizeof(T); };

	void write_s(const char *s)
	{ 
		int size = strlen(s) + 1;
		write(size);
		memcpy(&data[byte_size], s, size); 
		byte_size += size; 
	};

	void read_s(char *s, int &pos) const
	{ 
		int size;
		read(size, pos);
		memcpy(s, &data[pos], size); pos += size;
	};

	template <class T> 
	void write_array(T *s, int size) 
	{ memcpy(&data[byte_size], s, size); byte_size += sizeof(T)*size; };

	template <class T> 
	void read_array(T *s, int size, int &pos) const
	{ memcpy(s, &d[pos], size); pos += sizeof(T)*size; };

};
typedef std::vector<ptlblock> ptlblocks; 

struct ptlblock_dispatcher
{
	typedef CBFunctor2wRet<bool /*true for write*/, ptlblocks &, int /*return blocks processed*/>	handler_callback;

	static bool register_handler(const char *, handler_callback cb);
	static int	read_block(const ptlblock &block);
	static int	write_blocks(ptlblocks &blocks);
	static int  write_block(ptlblock &block);
	static void free_blocks(ptlblocks &blocks);
private:
	typedef Loki::AssocVector<const char*, handler_callback, str_cmp> HANDLERMAP;

	inline static HANDLERMAP &map()
	{
		static HANDLERMAP _m;
		return _m;
	}
};
#endif
