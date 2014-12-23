#pragma once

#include <pt/datatreeBranch.h>

namespace pt
{
namespace datatree
{
//-----------------------------------------------------------------------------
struct CountBranchVisitor
{
	CountBranchVisitor() : counter(0){}

	bool operator ()(const Branch *b) 
	{
		++counter;
		b->visitBranches(*this);
		return true;
	}
	int counter;
};
//-----------------------------------------------------------------------------
struct CopyBranchVisitor
{
	CopyBranchVisitor(Branch *destination) : 
	dest(destination), src_root_level(-1) {}

	bool operator()(const NodeID &nodeId, const Blob *blob)
	{
		dest->addBlob( nodeId, blob->_size, blob->_data, true, true);
		return true;
	}

	void operator ()(NodeID nid, const Node *n) 
	{ 
		dest->addNode(nid, *n); 
	}

	bool operator ()(const Branch *s) 
	{ 
		if (src_root_level < 0)
		{
			src_root_level = s->level();
			dest_root_level = dest->level();
		}
		int slevel = s->level() - src_root_level;

		while (slevel < (dest->level() - dest_root_level))
		{
			dest = dest->parent();
		};
		dest = dest->addBranch(s->id());
		dest->copyFlags(s);
		s->visitBlobs(*this);

		return true;
	}

	Branch  *dest;
	int		src_root_level;
	int		dest_root_level;
};
}
}