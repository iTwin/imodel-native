//-----------------------------------------------------------------------------
// DatatreeBranch.h
//
// Datatree is a Binary tree based structure for arbitrary run-time structures
// for run-time use and persistence
//
// Branch is the main class for datatree 
//-----------------------------------------------------------------------------
// Copyright Pointools Ltd 2003-2011 All rights reserved
//-----------------------------------------------------------------------------
#pragma once


// PT includes
#include <pt/variant.h>
#include <pt/classes.h>

// Pointools Vortex SDK does not use Parameter Map
#ifndef DATATREE_NO_PARAMMAP
	#include <pt/parametermap.h>
#endif

#include <pt/datatreeBlob.h>
#include <pt/datatreeNode.h>

namespace pt
{
namespace datatree
{
	// Object 
	//! Branch
	//! Tree branch object, an organizational structure to manage data Nodes
	class Branch
	{
	public:
		Branch(const NodeID &nid);

		virtual ~Branch();

		void			operator = (const Branch &br);							// deep copy. Do not use on subbranches

		const NodeID	&id() const					{ return _id; }				// Node ID (string identifier)
	
		bool			setID(const NodeID &nid);								// set Node ID, will fail if ID not null

		uint8			level() const				{ return _level; }			// level in tree

		Branch			*parent()					{ return _parent; }			// get parent branch

		const Branch	*parent() const				{ return _parent; }			// get const parent branch

		uint8			flags(int index=0) const	{ return _flags[index]; }	// arbitrary flags

		void			copyFlags(const Branch *br);							// copy flags from another branch
	
		void			setFlags(int index, uint8 val)	{ _flags[index] = val; }

		void			clear();												// clear all nodes, branches, blobs etc
		
		// Nodes
		int				numNodes() const		{ return (int)_nodes.size(); }	// num of nodes (exc branches) contained

		template <class T> 
		bool			setNodeValue(const NodeID &nid, T &v);					// update the value of a node								

		template <class T> 
		bool			addNode(NodeID id, T v);								// add a new node
		
		bool			addNode(const NodeID &nid, const Node &n);				// add a node copy

		bool			addNodeV(const NodeID &nid, const Variant &v);			// add a node

		template <class T> 
		bool			getNode(NodeID id, T &v) const;							// get a node of type T. false if types don't match or node does not exist

		bool			getNodeV(NodeID id, Variant &v) const;					// get a node. false if node does not exist

		const Node		*getNode(NodeID nodeid) const;							// get a node object


		// Blobs, arbitrary binary data objects
		int				numBlobs() const;

		bool			addBlob(NodeID id, uint32 size, const void* data,		// add arbitrary binary data blob
								bool copy=false, bool del=false, bool compress=false);

		Blob			*getBlob(NodeID id, bool decompress=true);				// get arbitrary binary data blob

		const Blob		*getBlob(NodeID id, bool decompress=true) const;		// get arbitrary binary data blob

		const Blob		*getBlob(int index, NodeID &id, bool decompress=true) const; // get arbitrary binary data blob by index

		// Sub-branches
		int				numBranches() const;									// number of branches, non-recursive

		int				numBranchesRecursive() const;							// number of branches recursively

		Branch			*getNestedBranch( const NodeID &nid0, const NodeID &nid1);											// easy access to nested branches

		Branch			*getNestedBranch( const NodeID &nid0, const NodeID &nid1, const NodeID &nid2);						// easy access to nested branches

		Branch			*getNestedBranch( const NodeID &nid0, const NodeID &nid1, const NodeID &nid2, const NodeID &nid3);	// easy access to nested branches

		bool			addBranchCopy(const Branch *src);						// add a copy of an existing branch

		Branch			*getBranch(const NodeID &nid, bool allow_create=false);	// get a sub-branch

		Branch			*getBranch(const NodeID &nid) const;					// get a const sub-branch

		const Branch	*getNthBranch(int n) const;								// get a const sub-branch by position

		Branch			*getNthBranch(int i);									// get a sub-branch by position

		Branch			*getIndexedBranch(int i);								// get an indexed branch with a numerical (not string) NodeID. Used for Arrays

		const Branch	*getIndexedBranch(int i) const;							// get an indexed branch with a numerical (not string) NodeID. Used for Arrays

		Branch			*addIndexedBranch();									// adds a branch with a numerical identifier, easy for arrays;

		Branch			*addBranch(const NodeID &nid);							// adds a branch this string identifier, more commonly used;

		bool			removeBranch(const NodeID &nid);						// remove a sub-branch

		Branch			*findBranch(const NodeID &nid);							// recursive search for a Branch by its Node ID

		const Branch	*findBranch(const NodeID &nid) const;					// recursive search for a Branch by its Node ID

		// Visitors
		template <class Visitor>
		void			visitNodes(Visitor &V, bool recursive = true, 
			bool self = true);													// visit Nodes using visitor object

		template <class Visitor>
		void			visitNodes(Visitor &V, bool recursive = true, 
			bool self = true) const;											// visit Nodes using visitor object

		template <class Visitor>
		void			visitBranches(Visitor &V);								// visit Branches using visitor object

		template <class Visitor>
		void			visitBranches(Visitor &V) const;						// visit Branches using visitor object
	
		template <class Visitor>
		void			visitBlobs(Visitor &V);									// visit Blobs using visitor object

		template <class Visitor>
		void			visitBlobs(Visitor &V) const;							// visit Branches using visitor object

#ifndef DATATREE_NO_PARAMMAP

		bool			addNodes(const ParameterMap &params);					// add nodes from a Parameter Map, non-recursive

		bool			getNodes(ParameterMap &params);							// copy Nodes into a Parameter Map, non-recursive

#endif

	protected:
		
		typedef std::map<NodeID, Branch*>	Branches;

		Branches		_branches;
		uint8			_flags[3];

	private:

		friend struct	Reader;

		typedef std::map<NodeID, Blob*>		Blobs;
		typedef std::map<NodeID, Node*>		NodeMap;
//		typedef std::map<NodeID, Array*>	Arrays;

		Blobs			_blobs;
		NodeMap			_nodes;
		NodeID			_id;
		uint8			_level;
		Branch			*_parent;
	};
}	// namespace datatree
}	// namespace pt 

#include <pt/datatreeBranch.in>