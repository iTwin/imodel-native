/*--------------------------------------------------------------------------*/ 
/*	Pointools Block															*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 28 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef PTLBLOCK_DEFINITION_HEADER
#define PTLBLOCK_DEFINITION_HEADER	1

#include <loki/assocvector.h>


#define PTLBLOCK_DEL_AFTER_USE	1

#ifdef PTL_EXPORTS
#define PTL_API EXPORT_ATTRIBUTE
#else
	#ifdef POINTOOLS_API_INCLUDE
		#define PTL_API 
	#else
		#define PTL_API IMPORT_ATTRIBUTE
	#endif
#endif

#define PTL_CHUNK_SIZE	4096

namespace ptl
{
//
// Data Chunk - expandable write buffer
//
class Chunk
{
public:
	/*allocate*/ 
	Chunk(Chunk **chunk /*chunk pointer*/, size_t s = PTL_CHUNK_SIZE)
	{
		current = chunk;
		(*current) = this;
		_chunk = new unsigned char[s];
		available = s;
		pos = 0;
		next = 0;
	}
	/*don't allocate*/ 
	Chunk(void* buffer, size_t sz)
	{
		_chunk = reinterpret_cast<unsigned char*>(buffer);
		available = sz;
		pos = 0;
		current = 0;
		next = 0;
	}
	Chunk(size_t sz = PTL_CHUNK_SIZE)
	{
		current = 0;
		_chunk = new unsigned char[sz];
		available = sz;
		pos = 0;
		next = 0;
	}
	~Chunk()
	{
		if (current) delete [] _chunk;
		if (next) delete next; 
	}
	/*expandable write*/ 
	void write(const void *buffer, size_t bytes)
	{
		assert(current);

		if (*current != this) 
		{
			(*current)->write(buffer, bytes);
			return;
		}
		if (available >= bytes)
		{
			memcpy(&_chunk[pos], buffer, bytes);
			pos += bytes;
			available -= bytes;
		}
		else 
		{
			attachChunk(bytes);
			(*current)->write(buffer, bytes);
		}
	}
	/*read from fixed size*/ 
	void read(void *buffer, size_t bytes)
	{
		//assert(!current);	
		memcpy(buffer, &_chunk[pos], bytes);
		pos += bytes;
	}
	void peek(void *buffer, size_t bytes)
	{
		//assert(!current);	
		memcpy(buffer, &_chunk[pos], bytes);
	}
	/*size calc*/ 
	size_t listsize() const
	{
		int s = 0;
		const Chunk *c =this;

		while (c)
		{
			s += c->size();
			c = c->next;
		}
		return s;
	}
	/*size calc*/ 
	size_t listtotalsize() const
	{
		int s = 0;
		const Chunk *c =this;

		while (c)
		{
			s += c->totalSize();
			c = c->next;
		}
		return s;
	}
	size_t numChunks() const
	{
		int n = 0;
		const Chunk *c =this;

		while (c) 
		{
			n++;
			c = c->next;
		}
		return n;
	}
	size_t size() const { return pos+1; }
	size_t totalSize() const { return available; }

	unsigned char* _chunk;

	size_t available;
	size_t pos;

	Chunk *next;

protected:
	void attachChunk(size_t minsize)
	{
		assert(current);	
		next = new Chunk(current, minsize > PTL_CHUNK_SIZE ? minsize : PTL_CHUNK_SIZE);
	}
	Chunk **current;
};
//
// Block
//
class Block
{
public:
	PTL_API static Block* allocBlock(void *buffer, size_t sz);
	PTL_API static Block* allocBlock(size_t sz);
	PTL_API static Block* allocBlock();
	PTL_API static void freeBlock(Block* b);

	char			identifier[8];
	unsigned char	version;
	Chunk			*firstchunk() { return chunk; }
private:
	Chunk			*chunk;
	Chunk			*endchunk;
protected:
	/*existing data*/ 
	Block(void *buffer, size_t sz) 	{ chunk = new Chunk(buffer, sz);}

	/*known size write/read buffer*/ 
	Block(size_t sz) {	chunk = new Chunk(sz); };

	/*expanding chunk buffer*/ 
	Block() 	{ chunk = new Chunk(&endchunk); }
	~Block()	{ if (chunk) delete chunk; }

	void clearChunks()			{ if (chunk) delete chunk; endchunk = 0; chunk = 0; }
	void addChunk(size_t size)	
	{ 
		if (endchunk) endchunk->next = new Chunk(size);
		else chunk = endchunk = new Chunk(size);
	}
	Chunk			*lastchunk()		{ return endchunk; }
public:
	template <class T>		void write(const T &s)		{ chunk->write(&s, sizeof(T)); }
	void write_s(const char *s) { short len = strlen(s) + 1; write(len); chunk->write(s, len); }
	template <class T>	void write_array(const T *s, int sz) { chunk->write(s, sz*sizeof(T)); };
	void write_chunk(const void *d, int bytes) { write(bytes); chunk->write(d, bytes); }
	void write(const void* d, int bytes) const { chunk->write(d, bytes); }

	template <class T>		void read(T &s) const { chunk->read(&s, sizeof(T)); }
	void read(void* d, int bytes) const { chunk->read(d, bytes); }

	void read_s(char *s) const	{ short len; read(len); chunk->read(s, len); }
	template <class T> 	void read_array(T *s, int sz) const	{ chunk->read(s, sz*sizeof(T)); }
	
	void read_chunk(void *d) const { int bytes; read(bytes); chunk->read(d, bytes); }
	void get_chunksize(int &size) const { chunk->peek(&size, sizeof(int)); }
		
	/* sub block processing */ 
	void write_block(Block *b);
	Block *read_block() const;

	size_t size() const	{ return chunk->listsize(); }
	size_t sizeLeft() const	{ return chunk->totalSize() - chunk->listsize() + 1; }
};

typedef std::vector<Block*> Blocks; 

struct id_cmp
{
    typedef const char* first_argument_type;

    bool operator () (const char* a, const char* b) const
    {
		for (int i=0; i<7; i++)
		{
			if (a[i] < b[i]) return true;
			if (a[i] > b[i]) return false;
		}
		return false;
    }
};
//
// Block Handler
//
class Handler
{
public:
	typedef std::function<Block *()>				write_cb;
	typedef std::function<bool(const Block *)>	read_cb;

	PTL_API Handler(const char *id, read_cb rcb, write_cb wcb, bool configuration = false, const char *descriptor=0);
	PTL_API ~Handler();
	char		identifier[8];
	write_cb	writeblock;
	read_cb		readblock;
	bool		configuration;
	char		descriptor[32];
};
typedef std::function<void()>	clear_cb;
} 
#endif