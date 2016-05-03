/*--------------------------------------------------------------------------*/ 
/*	Memory Manager class definition											*/ 
/*  (C) 2003 -2004 Copyright Pointools Ltd, UK - All Rights Reserved		*/ 
/*																			*/ 
/*  Last Updated 27 June 2004 Faraz Ravi									*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef POINTOOLSPAGEABLE_MEMORY_MANMAGER_CLASS_DEFINITION
#define POINTOOLSPAGEABLE_MEMORY_MANMAGER_CLASS_DEFINITION

#include <ptclasses/Pageable.h>

namespace pt;

/* == Singleton Thread Safe Pageable Physical Memory Manager ==
	lagged memory manager with delayed memory free
	to optimise reallocation */

/* designed for few objects (<5000) of large size (200k-> 3Mb)*/

class PageableMemoryManager
{
public:
	PageableMemoryManager();
	~PageableMemoryManager();

	/*alloc memory for requested lod*/
	/*existing memory will be freed*/
	void p_alloc(Pageable *p);

	/*free memory*/
	void p_free(Pageable *p);

	/*resize current allocation*/
	/*existing alloc within bounds left intact*/
	void p_resize(Pageable *p);

	/*Memory useage in Bytes*/
	int useage() const;

	//register a handler for useage event*/
	void registerUseageEvent(bool inc, float physical_mem_useage, MemUseageCB handler);

	/*switch on/off profiling of memory useage*/
	void logProfile(bool log);

	/* delete all allocations*/
	void clear();

private:
	/*Sub memory manager for sub block size*/
	struct BlockInfo
	{
		int		timestamp;
		int		size;
	};
	struct Block : public BlockInfo
	{
		void *mem;
	};
	class MemMan
	{
	public:
		MemMan();
		~MemMan();

		bool alloc_mem(Pageable *p);
		bool free_mem(Pageable *p);
		
	private:
		/*map of block data keyed by allocation pointer*/
		std::map<void*,BlockInfo*>	m_blocks;
		/*queue of freed blocks available for relocation*/
		std::queue<Block*>			m_freed; 

		void clear();
	};
		
	/*Memory Managers*/
	

	
	float	m_buffer_coef;
};
#endif