#pragma once

#include <pt/datatree.h>
#include <ptfs/filepath.h>
#include <ptedit/editNodeDef.h>
#include <fastdelegate/fastdelegate.h>
#include <ptcmdppe/cmdprogress.h>

namespace ptedit
{
struct StateHandler
{
	typedef pt::FastDelegate1< pt::datatree::Branch *>			WriteCB;	// write state callback
	typedef pt::FastDelegate1< const pt::datatree::Branch *>	ReadCB;		// read state callback

	WriteCB write;
	ReadCB	read;
};

class OperationStack
{
public:

	/* constructor */ 
	OperationStack();

	/* persistance constructor */ 
	OperationStack( const pt::datatree::Branch *b );

	/* destructor */ 
	~OperationStack();

	/* execute the operations stack on the engine data */ 
	void execute( bool pauseengine= true );
	
	/* global state management*/ 
	static void addStateHandler( StateHandler *h );

	/* visitor that applys the filter operation */ 
	void operator ()(pt::datatree::Branch *b);
	
	/* write the current global state to branch */ 
	void writeStateBranch( pt::datatree::Branch *b );

	/* restore the current state from a node */ 
	bool readStateBranch( const pt::datatree::Branch *b );

	/* add an edit operation to the stack */ 
	bool addOperation( EditNodeDef *node, bool apply=true, bool group=false);

	/* add an edit operation to the stack by name */ 
	bool addOperation( const char*node, bool apply=true, bool group=false);

	/* close an edit node group */ 
	void closeGroup();

	/* clear all operations */ 
	void clear();

	/* remove an operation */ 
	void removeBranch(pt::datatree::Branch *b);	

	/* read an entire operations tree */ 
	void readStack(const pt::datatree::Branch *b, bool merge=false);

	/* read an entire operations tree */ 
	void writeStack( pt::datatree::Branch *b );

	/* remove operations from the stack by flag */ 
	int removeOperationsByFlag( uint flags );

	/* return a const pointer to the stack datatree */ 
	const pt::datatree::Branch *stack() const { return m_operations; }

	/* evaluate the stack on the currently loaded point clouds and save to edit file */ 
	bool evaluateToFile( const pt::String &name, const ptds::FilePath &file, bool collapse );

	/* read an evaluated stack file - clears stack and pushs single node on */ 
	bool readEvaluated( const ptds::FilePath &file );

	/* return an integer representing the current state, session lifetime only  - useful for invaliding caches etc*/ 
	int	stateId(void) const { return m_stateId ; }

private:

	/* generate the next key */ 
	const char* generateNextKey();
	
	/* create a new operations branch */ 
	pt::datatree::Branch* newOpBranch();

	/* write an editnode to a branch */ 
	void writeNodeData( EditNodeDef *node, pt::datatree::Branch *b );

	/* tree of operations */ 
	pt::datatree::Branch *m_operations;

	/* subbranch used for group handling */ 
	pt::datatree::Branch *m_subbranch;
	
	bool m_group;	// current operation is part of a group
	
	/* last state for state redundency checking */ 
	pt::datatree::Branch m_lastState;

	/* ui feedback */ 
	ptapp::CmdProgress *m_progress;
	
	int m_lastOpKey;

	int m_stateId;
};
}