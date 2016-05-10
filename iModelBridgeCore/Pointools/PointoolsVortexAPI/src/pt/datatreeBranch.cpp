//-----------------------------------------------------------------------------
// DatatreeBranch.cpp
//
// Datatree is a Binary tree based structure for arbitrary run-time structures
// for run-time use and persistence
//
// Branch is the main class for datatree 
//-----------------------------------------------------------------------------
// Copyright Pointools Ltd 2003-2011 All rights reserved
//-----------------------------------------------------------------------------
#include "PointoolsVortexAPIInternal.h"
#include <pt/datatreeBranch.h>
#include <pt/datatreeUtils.h>

namespace pt
{
namespace datatree
{

Branch::Branch( const NodeID &nid ) : _id(nid), _level(0), _parent(0)	
{
	memset(_flags, 0, 3);
}
//-----------------------------------------------------------------------------
Branch::~Branch()
{
	clear();
}
//-----------------------------------------------------------------------------
Branch * Branch::getNestedBranch( const NodeID &nid0, const NodeID &nid1 )
{
	Branch *br = getBranch(nid0);
	if (br)
	{
		Branch *br1 = br->getBranch(nid1);
		if (br1) return br1;
	}
	return 0;
}
//-----------------------------------------------------------------------------
Branch * Branch::getNestedBranch( const NodeID &nid0, const NodeID &nid1, const NodeID &nid2 )
{
	Branch *br = getNestedBranch(nid0, nid1);
	if (br)
	{
		Branch *br2 = br->getBranch(nid2);
		if (br2) return br2;
	}
	return 0;
}
//-----------------------------------------------------------------------------
Branch * Branch::getNestedBranch( const NodeID &nid0, const NodeID &nid1, const NodeID &nid2, const NodeID &nid3 )
{
	Branch *br = getNestedBranch(nid0, nid1, nid2);
	if (br)
	{
		Branch *br3 = br->getBranch(nid3);
		if (br3) return br3;
	}
	return 0;
}
//-----------------------------------------------------------------------------
Branch * Branch::getBranch( const NodeID &nid, bool allow_create/*=false*/ )
{
	/*may already exist, just return existing*/ 
	Branches::iterator i = _branches.find(nid);
	if (i != _branches.end())
		return i->second;
	else if (allow_create)
	{
		Branch *b = new Branch(nid);
		b->_parent = this;

		_branches.insert(Branches::value_type(nid, b)).second;
		if (b) b->_level = _level + 1;
		return b;
	}
	else return 0;
}
//-----------------------------------------------------------------------------
Branch * Branch::getBranch( const NodeID &nid ) const
{
	/*may already exist, just return existing*/ 
	Branches::const_iterator i = _branches.find(nid);
	if (i != _branches.end())
		return i->second;
	else return 0;
}
//-----------------------------------------------------------------------------
void Branch::copyFlags( const Branch *br )
{
	_flags[0] = br->flags(0);
	_flags[1] = br->flags(1);
	_flags[2] = br->flags(2);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool Branch::addBranchCopy( const Branch *src )
{
	CopyBranchVisitor v(this);
	src->visitNodes(v);
	return true;
}
//-----------------------------------------------------------------------------
Branch* Branch::addBranch( const NodeID &nid ) /* adds a branch this string identifier, more commonly used */
{
	Branch *br = new Branch(nid);

	br->_level = _level + 1;
	br->_parent = this;

	if (_branches.insert(Branches::value_type(nid,br)).second)
		return br;
	else
	{
		delete br;
		return _branches.find(nid)->second;
	}
}
//-----------------------------------------------------------------------------
void Branch::clear()
{
	NodeMap::iterator n = _nodes.begin();
	while (n != _nodes.end())
	{
		delete n->second;
		++n;
	};
	_nodes.clear();

	Branches::iterator b = _branches.begin();
	while (b != _branches.end())
	{
		delete b->second;
		++b;
	}

	_branches.clear();

	Blobs::iterator bl = _blobs.begin();
	while (bl != _blobs.end())
	{
		delete bl->second;
		++bl;
	}
	_blobs.clear();
	memset(_flags, 0, 3);
}
//-----------------------------------------------------------------------------
int Branch::numBranchesRecursive() const
{
	CountBranchVisitor v;
	visitBranches(v);

	return v.counter;
}
//-----------------------------------------------------------------------------
void Branch::operator=( const Branch &br )
{
	assert(!_parent);
	clear();
	Branches::const_iterator it = br._branches.begin();
	while (it != br._branches.end())
	{
		addBranchCopy(it->second);
		++it;
	}
	_id = br._id;
}
//-----------------------------------------------------------------------------
bool Branch::addNode( const NodeID &nid, const Node &n )
{
	Node *nn = Node::create( n );
	bool res = (_nodes.insert(NodeMap::value_type(nid, nn)).second);
	if (!res) delete nn;
	return res;
}
//-----------------------------------------------------------------------------
bool Branch::addNodeV( const NodeID &nid, const Variant &v )
{
	Node *nn = Node::create( v );
	
	bool res = (_nodes.insert(NodeMap::value_type(nid, nn)).second);
	if (!res) delete nn;
	return res;
}
//-----------------------------------------------------------------------------
bool Branch::getNodeV( NodeID id, Variant &v ) const
{
	NodeMap::const_iterator it = _nodes.find(id);
	if (it != _nodes.end())
	{
		v = it->second->var();
		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
const Node * Branch::getNode( NodeID nodeid ) const
{
	NodeMap::const_iterator it = _nodes.find(nodeid);
	return (it != _nodes.end()) ? it->second : 0;
}
//-----------------------------------------------------------------------------
bool Branch::addBlob( NodeID id, uint32 size, const void* data, bool copy/*=false*/, bool del /*= false*/, bool compress/*=false*/ )
{
	uint8 *dt = (uint8*)data;

	if (copy || compress)
	{
		try
		{
			dt = new uint8[size];
			memcpy(dt, data, size);
		}
		catch (std::bad_alloc)
		{
			return false;
		}
	}
	Blob *blob = new Blob(dt, size, copy ? true : del);

	if (compress)
		blob->compress(Blob::CompressSCZ);

	// if it already exists overwrite
	Blobs::iterator i = _blobs.find(id);
	if (i!=_blobs.end()) 
	{
		if (copy)
			delete [] i->second;
		i->second = blob;
	}	
	else
	{
		_blobs.insert(Blobs::value_type(id, blob));
	}
	return true;
}
//-----------------------------------------------------------------------------
Blob * Branch::getBlob( NodeID id, bool decompress )
{
	Blobs::iterator it = _blobs.find(id);
	if (it != _blobs.end())
	{
		if (decompress) it->second->decompress();	
		return it->second;
	}
	return 0;
}
//-----------------------------------------------------------------------------
const Blob * Branch::getBlob( NodeID id, bool decompress ) const
{
	Blobs::const_iterator it = _blobs.find(id);
	if (it != _blobs.end())
	{
		if (decompress) static_cast<Blob*>(it->second)->decompress();		
		return it->second;
	}
	return 0;
}
//-----------------------------------------------------------------------------
const Blob * Branch::getBlob( int index, NodeID &id, bool decompress ) const
{
	Blobs::const_iterator it = _blobs.begin();
	int i=0;
	while (it != _blobs.end())
	{
		if (i++==index)
		{
			id = it->first;
			if (decompress) static_cast<Blob*>(it->second)->decompress();		
			return it->second;
		}
		++it;
	}
	return 0;
}
//-----------------------------------------------------------------------------
const Branch * Branch::getNthBranch( int n ) const
{
	if ((int)_branches.size() <= n) return 0;

	Branches::const_iterator it = _branches.begin();

	for (;n>0;n--)
		if (it != _branches.end()) ++it;

	return it->second;
}
//-----------------------------------------------------------------------------
Branch * Branch::getNthBranch( int n )
{
	if ((int)_branches.size() <= n) return 0;

	Branches::iterator it = _branches.begin();

	for (;n>0;n--)
		if (it != _branches.end()) ++it;

	return it == _branches.end() ? 0 : it->second;
}
//-----------------------------------------------------------------------------
Branch * Branch::getIndexedBranch( int i )
{
	char buff[NODE_ID_SIZE];

	NodeID nid(itoa(i, buff, 10));
	return getBranch( nid );
}
//-----------------------------------------------------------------------------
const Branch * Branch::getIndexedBranch( int i ) const
{
	char buff[NODE_ID_SIZE];

	NodeID nid(itoa(i, buff, 10));
	return getBranch( nid );
}
//-----------------------------------------------------------------------------
Branch* Branch::addIndexedBranch() /* adds a branch with a numerical identifier, easy for arrays */
{
    int num = static_cast<int>(_branches.size() + 1);
	char buff[NODE_ID_SIZE];
	NodeID numID(itoa(num, buff, 10) );
	return addBranch( numID );
}
//-----------------------------------------------------------------------------
Branch* Branch::findBranch( const NodeID &nid )
{
	Branches::iterator f = _branches.find(nid);
	return f == _branches.end() ? 0 : f->second;
}
//-----------------------------------------------------------------------------
const Branch* Branch::findBranch( const NodeID &nid ) const
{
	Branches::const_iterator f = _branches.find(nid);
	return f == _branches.end() ? 0 : f->second;
}
//-----------------------------------------------------------------------------
bool Branch::removeBranch( const NodeID &nid )
{
	Branches::iterator i = _branches.find(nid);
	if (i == _branches.end()) return false;
	delete i->second;
	_branches.erase(i);
	return true;
}
//-----------------------------------------------------------------------------
bool Branch::setID( const NodeID &nid )
{
	if (!_id) 
	{ 
		_id = nid; 
		return true; 
	} 
	return false;
}
//-----------------------------------------------------------------------------
int Branch::numBranches() const
{
	return (int)_branches.size();
}
//-----------------------------------------------------------------------------
int Branch::numBlobs() const
{
	return (int)_blobs.size();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DATATREE_NO_PARAMMAP
struct ReadParamsVisitor
{
	ReadParamsVisitor(Branch *b) : branch(b) {};

	ParamIdType identifier;
	Branch *branch;

	template <class T>
	void operator()(const T &v)
	{
		branch->addNode(identifier.c_str(), v);
	}
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct WriteParamsVisitor
{
	WriteParamsVisitor(Branch *b, ParameterMap *p) : branch(b), params(p) {};

	ParameterMap *params;
	ParamIdType identifier;
	Branch *branch;

	template <class T>
	void operator()(const T &v)
	{
		T bv;
		if (branch->getNode(identifier.c_str(), bv))
			params->set(identifier, bv);
	}
};
//-----------------------------------------------------------------------------
bool Branch::addNodes( const ParameterMap &params )
{
	ReadParamsVisitor rp(this);
	params.applyConstVisitor(rp);
	return true;
}
//-----------------------------------------------------------------------------
bool Branch::getNodes( ParameterMap &params )
{
	WriteParamsVisitor rp(this, &params);
	params.applyVisitor(rp);
	return true;
}
//-----------------------------------------------------------------------------
#endif	// PARAMETER MAP
}	// namespace datatree
}	// namespace pt