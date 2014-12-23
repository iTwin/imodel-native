//-----------------------------------------------------------------------------
// Datatree.h
//
// Datatree is a Binary tree based structure for arbitrary run-time structures
// for run-time use and persistence
//
//-----------------------------------------------------------------------------
// Copyright Pointools Ltd 2003-2011 All rights reserved
//-----------------------------------------------------------------------------
#pragma once

#include <pt/datatreeMeta.h>
#include <pt/datatreeBlob.h>
#include <pt/datatreeBranch.h>

#ifndef DATATREE_NO_PARAMMAP
	#include <pt/parametermap.h>
#endif

#ifdef _DEBUG
  #define DATATREE_DEBUGGING
#endif

#include <pt/datatreeBranch.h>

namespace pt
{
namespace datatree
{
	//
	// data tree
	//
	class DataTree : public Branch
	{
	public:
		/* build / write constructor */ 
		DataTree()  : Branch("Root"), _readonly(false) {};

		void	setReadOnly(bool read=true)	{ _readonly = read; }
		bool	getReadOnly() const			{ return _readonly; }

		template <class T>
		bool	addNode(const NodeID &nid, T v)
		{
			if (_branchstack.size())
				return _branchstack.top()->addNode(nid, v);
			else return Branch::addNode(nid, v);
		}
		template <class T>
		bool	addNode(const NodeID &nid, const T &v)
		{
			if (_branchstack.size())
				return _branchstack.top()->addNode(nid, v);
			else return Branch::addNode(nid, v);
		}
		template <class T>
		bool	getNode(NodeID nid, T &v) const
		{
			if (_branchstack.size())
				return _branchstack.top()->getNode(nid, v);
			else return Branch::getNode(nid, v);		
		}

		// used to write datatree - maybe remove if not used
		void	root();

		int		pushBranch(const NodeID &nid);

		int		popBranch();

		Branch* currentBranch();

		bool	addBranch(const Branch *br);	


		template <class T>
		bool	addNode(const NodeID &nid, const T *v, int num)
		{
			if (_branchstack.size())
				return _branchstack.top()->addNode(nid, v, num);
			else return Branch::addNode(nid, v, num);
		}	

#ifndef DATATREE_NO_PARAMMAP
		bool	addNodes(const ParameterMap &pm);
#endif
		
		// IO
		// this is the original implementation for read/write 
		// datatree. But it uses fread and does not offer strong
		// fwd compatibility
		bool	writeTree(const String &path);

		// read from file
		bool	readTree(const String &filepath);

		// construct from file
		DataTree(const String &path);

		// check file type
		static bool isDTreeFile(const String &filepath);
		
	private:
		std::stack<Branch*>		_branchstack;
		bool					_readonly;
	};
} //namespace datatree
} // namespace pt
