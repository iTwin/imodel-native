#include "PointoolsVortexAPIInternal.h"
#include <ptedit/editStack.h>
#include <ptedit/editState.h>
#include <ptedit/editStackState.h>
#include <ptedit/editEvaluatedStackNode.h>

#include <pt/datatreeUtils.h>

#include <ptl/project.h>

namespace ptedit
{
	/* global state handling */ 
	std::vector<StateHandler *> g_stateHandlers;
}
using namespace pt;
using namespace ptedit;

//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
OperationStack::OperationStack() : m_lastState("Selection") 
{
	try
	{
		m_operations = new datatree::Branch("Selection");	// new in constructor is not good
	}
	catch(...) 
	{ 
		assert(0); 
		m_operations = 0;
	}

	m_group = false;
	m_subbranch =0;
	m_progress = 0;
	m_lastOpKey = 0;
	m_stateId = 0;
}
//-----------------------------------------------------------------------------
// constructor from branch
//-----------------------------------------------------------------------------
OperationStack::OperationStack( const pt::datatree::Branch *b ) : m_lastState("Selection") 
{
	try
	{
	m_operations = new datatree::Branch("Selection");	// new in constructor is not good
	}
	catch(...) 
	{ 
		assert(0); 
		m_operations = 0;	
	}

	m_group = false;
	m_subbranch =0;
	m_progress = 0;
	m_lastOpKey = 0;
	m_stateId = 0;

	readStack(b);	
}
OperationStack::~OperationStack()
{
	try
	{
		delete m_operations;
	}
	catch (...){}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool OperationStack::addOperation( const char *node, bool apply, bool group)
{
	return addOperation( EditNodeDef::findNodeDef( node ), apply, group );
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool OperationStack::addOperation( EditNodeDef *node, bool apply, bool group)
{
	if (!node) return false;

	ptl::Project::project()->modify();

	/* state does not change within group*/ 
	if (!m_group) writeStateBranch( newOpBranch() );

	++m_stateId;	/* mostly to check for changes - session lifetime, never reset */ 

	/* first item in group */ 
	if (group && !m_group)
	{
		datatree::Branch *pb = newOpBranch();
		if (pb)
		{
			writeNodeData( node, pb );

			m_group = true;			
			m_subbranch = pb->addBranch("group");
			++m_lastOpKey;
		}
		else return false; /* error */ 
	}
	else if (!group && m_group)
	{
		m_subbranch = 0;
		m_group = false;
	}

	/* create the node branch */ 
	datatree::Branch *b = newOpBranch();
	writeNodeData( node, b );

	/* apply this operation */ 
	if (apply) (*this)(b);

	return true;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct RemoveFlagged
{
	RemoveFlagged(uint flags) 
	{
		m_flags = flags;
		numRemoved = 0;
	}
	void operator ()(datatree::Branch *b)
	{
		String id;

		datatree::Branch *fb = b;

		if (!b->getNode("operation", id))
		{
			fb = b->getNthBranch(0);
			if (fb) id = fb->id().get();
			else return; /* error */ 
		}

		EditNodeDef *op = EditNodeDef::findNodeDef( id.c_str() );	
		if (op) 
		{
			uint nf = op->flags();
			if ((nf & m_flags) == m_flags) 
			{
				numRemoved++;
				b->clear();
                pt::String str("null");
				b->setNodeValue("operation", str);
			}
		}
	}
	uint m_flags;
	int numRemoved;
};
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int OperationStack::removeOperationsByFlag( uint flags )
{
	RemoveFlagged v(flags);
	m_operations->visitBranches(v);
	return v.numRemoved;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OperationStack::addStateHandler( StateHandler *h )
{
	g_stateHandlers.push_back( h );
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void  OperationStack::closeGroup()
{
	String desc;
	if (m_subbranch)
	{
		m_subbranch->parent()->getNode("desc", desc);
		char num[8];
		sprintf(num, " [%d]", m_subbranch->numBranches());

		desc += String(num);

		m_subbranch->parent()->setNodeValue("desc", desc);
		m_subbranch = 0;
		
		/* all groups consolidate at the moment */ 
		EditNodeDef *op = EditNodeDef::findNodeDef( "Consolidate" );
		op->apply();
	}
	m_group = false;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OperationStack::clear()
{
	ptl::Project::project()->modify();

	delete m_operations;
	m_operations = new datatree::Branch("Selection");
	m_lastOpKey = 0;
	m_subbranch = 0;
}
//-----------------------------------------------------------------------------
// execute the operations in the stack
//-----------------------------------------------------------------------------
void OperationStack::execute( bool pauseengine )
{
	if (pauseengine) pointsengine::pauseEngine();

	datatree::Branch save("save");	// store state to ensure invariance
	writeStateBranch( &save );

	m_progress = new ptapp::CmdProgress("Applying edit", 0, m_operations->numBranches(), false);
	
	m_operations->visitBranches(*this);	// actual execution
	
	delete m_progress;
	m_progress = 0;
	m_subbranch = 0;

	readStateBranch( &save );		// restore state
	
	if (pauseengine) pointsengine::unpauseEngine();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OperationStack::writeStateBranch( datatree::Branch *b )
{
	int state = 102857; /* magic number */ 
	for (int i=0; i<g_stateHandlers.size();i++)
	{
		g_stateHandlers[i]->write( b );
	}
	b->addNode( "state", state );
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool OperationStack::readStateBranch( const datatree::Branch *b )
{
	int state;
	if (!b->getNode( "state", state ) || state != 102857)
		return false;

	for (int i=0; i<g_stateHandlers.size();i++)
	{
		g_stateHandlers[i]->read( b );
	}
	return true;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OperationStack::operator ()(datatree::Branch *b)
{
	static bool inGroup = false;

	String id;
	datatree::Branch *fb = b;

	/* group node ? */ 
	if (b->getBranch("group"))
	{
		inGroup = true;
		b->getBranch("group")->visitBranches(*this);
		m_progress->inc();
		inGroup = false;
		return;
	}
	/* or is this a state node ? */ 
	else if (readStateBranch(b)) return;
	
	if (!b->getNode("operation", id))
	{
		fb = b->getNthBranch(0);
		if (fb) id = fb->id().get();
		else return; /* error */ 
	}

	EditNodeDef *op = EditNodeDef::findNodeDef( id.c_str() );
	
	/* apply the operation */ 
	if (op)
	{
		bool mt = g_state.multithreaded;

		op->readState( fb );
		
		if ( mt && op->flags() & EditNodeMultithread )
			g_state.multithreaded = true;

		op->apply();

		/* consolidate if needed */ 
		if ( op->flags() & (EditNodePostConsolidateSel | EditNodePostConsolidateVis) && !inGroup )
		{
			EditNodeDef *op = EditNodeDef::findNodeDef( "Consolidate" );
			if (op) op->apply();
			
			EditNodeDef::findNodeDef( "SetLayer" );
			if (op) op->apply();
		}
		g_state.multithreaded = mt;
	}
	if (m_progress && !inGroup) m_progress->inc();
}
static const char* generateKeyString( int key )
{
	static char index[48];
	sprintf(index, "op %05d", key);
	return index;	
}
//-----------------------------------------------------------------------------
// Renumber ops visitor
//-----------------------------------------------------------------------------
struct RenumberOperationsVisitor
{
	RenumberOperationsVisitor( int &k ) : key(k) {
		key = 0;
	}
	bool operator ()(datatree::Branch *b)
	{
		if (b->getNode("key"))
			b->setNodeValue("key", key);

		/* is this an op code? */
		if ( b->id().get()[0] == 'o' && b->id().get()[1] == 'p'
			&& b->id().get()[2] == ' ' )

			b->setID( datatree::NodeID( generateKeyString(key)) );

		++key;
		return true;
	}
	void operator ()( const datatree::NodeID &id, const datatree::Node *n)
	{}

	int &key;
};
//-----------------------------------------------------------------------------
/* remove an operation */ 
//-----------------------------------------------------------------------------
void OperationStack::removeBranch(datatree::Branch *b)
{
	m_operations->removeBranch(b->id());
}	

//-----------------------------------------------------------------------------
/* generate the next key */ 
//-----------------------------------------------------------------------------
const char* OperationStack::generateNextKey()
{
	return generateKeyString(m_lastOpKey++);
}
//-----------------------------------------------------------------------------
/* create a new operations branch */ 
//-----------------------------------------------------------------------------
datatree::Branch* OperationStack::newOpBranch()
{
	const char *index = generateNextKey();
	if (!m_subbranch) return m_operations->addBranch(index);
	else return m_subbranch->addBranch(index);
}
//-----------------------------------------------------------------------------
/* write an editnode to a branch */ 
//-----------------------------------------------------------------------------
void OperationStack::writeNodeData( EditNodeDef *node, datatree::Branch *b )
{
	if (!m_group)
	{
		b->addNode("desc", node->desc() );
		b->addNode("icon", node->icon() );
		b->addNode("key", m_lastOpKey );
	}
	datatree::Branch *fb = b->addBranch( node->name().c_str() );
	node->writeState( fb );
}
//-----------------------------------------------------------------------------
/* read an entire operations tree */ 
//-----------------------------------------------------------------------------
void OperationStack::readStack(const datatree::Branch *b, bool merge)
{
	if (!merge) clear();

	datatree::CopyBranchVisitor v(m_operations);
	const datatree::Branch *src = b->findBranch("Selection");
	if (src) src->visitNodes(v, true, false);
	else
	{
		b->visitNodes(v, true, false);
	}

	RenumberOperationsVisitor r( m_lastOpKey );
	m_operations->visitNodes(r,true,false);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OperationStack::writeStack( datatree::Branch *b )
{
	b->addBranchCopy( m_operations );
}

//-----------------------------------------------------------------------------
/* evaluate the stack on the currently loaded point clouds and save to edit file */ 
//-----------------------------------------------------------------------------
bool OperationStack::evaluateToFile( const pt::String &name, const ptds::FilePath &file, bool collapse )
{
	g_editApplyMode = EditIntentRegen | EditIncludeOOC;	// full execution, including OOC data
	
	execute();	// do execute, point state should be cleared before this

	g_editApplyMode = EditNormal;


	EvaluatedOpStack::Write( pt::String("working"), file, this );

	/* create a node */ 
	if (collapse)
	{
		readEvaluated( file );
	}
	return true;
}
//-----------------------------------------------------------------------------
/* read an evaluated stack file - clears stack and pushs single node on */ 
//-----------------------------------------------------------------------------
bool OperationStack::readEvaluated( const ptds::FilePath &file )
{
	/* clear the stack first */ 
	clear();

	static EvaluatedStackNode s_evNode;

	s_evNode.setFilePath( file );
	s_evNode.setStateName( pt::String("working") );

	addOperation( &s_evNode, true );	

	return true;
}