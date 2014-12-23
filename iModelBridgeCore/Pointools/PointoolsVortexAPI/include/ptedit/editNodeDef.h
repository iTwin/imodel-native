#pragma once
#include <pt/datatree.h>
#include <pt/typedefs.h>

#include <map>

#define DECLARE_EDIT_NODE( n, d, i, f ) \
	const pt::String &name() const { static pt::String s(n); return (s); } \
	const pt::String &desc() const { static pt::String s(d); return (s); } \
	int icon() const { return (i); } \
	uint flags() { return (f); } 


namespace ptedit
{
//
// Copyright Pointools 2009
//
enum EditNodeFlags
{
	EditNodeAlgorithmOnly = 32,		/* an algorithm with no parameters or user input */ 
	EditNodeMultithread = 1,		/* allow multithreaded execution */ 
	EditNodePostConsolidateSel = 2,	/* consolidate voxel pnt selection */ 
	EditNodePostConsolidateVis = 4,	/* consolidate voxel pnt visibility */ 
	EditNodeSilent = 8,				/* don't display in tree */ 
	EditNodeGroupType = 16,			/* group together */ 
	EditNodeDoesPaint = 64
};
//
// EditNode Base class (interface)
//
// Anything that enters the Edit History is derived from this
// class which provides state loading and writing.
class EditNodeDef
{
protected:
	/* constructor performs self registration */ 
	EditNodeDef( const char*_name );
public:
	virtual const pt::String &name() const =0;
	virtual const pt::String &desc() const =0;
	virtual int icon() const { return -1; }

	/* apply the filter, return true if anything was done */ 
	virtual bool apply() { return false; };
	virtual bool writeState( pt::datatree::Branch *b) const { return true; };
	virtual bool readState(const pt::datatree::Branch *) { return true; };
	virtual uint flags() { return 0; }

	static bool applyNodeDef( const pt::datatree::Branch *b );
	static EditNodeDef *findNodeDef( const char *name );
	static bool applyByName( const char *name );

private:
	pt::datatree::Branch _lastState;
};
} /* end namespace */ 