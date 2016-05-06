#include "PointoolsVortexAPIInternal.h"
#include <ptedit/editState.h>
#include <ptedit/editStack.h>
#include <ptedit/editApply.h>

#include <ptedit/editmanager.h>
#include <ptengine/clipManager.h>

using namespace ptedit;
using namespace pt;


namespace ptedit
{
GlobalState g_state;
ubyte		g_activeLayers	= 1;
ubyte		g_visibleLayers = 1;
ubyte		g_currentLayer	= 1;
ubyte		g_lockedLayers	= 0;

uint		g_editApplyMode		= EditNormal;
uint		g_editWorkingMode	= EditWorkOnAll;
}
/* layer mask to point layer mask 			*/ 
/* point layers are shifted left one bit 	*/ 
static ubyte pointLayerMask( ubyte layerMask )
{
	ubyte pntLyr = (layerMask << 1);
	pntLyr &= ~1;
	pntLyr &= ~128;

	return pntLyr;
}
//
GlobalState::GlobalState() : layer(1,1,0,1)
{
	static StateHandler h;
    h.read = fastdelegate::MakeDelegate(this, &GlobalState::readState);
    h.write = fastdelegate::MakeDelegate(this, &GlobalState::writeState);
	
	constraint = 0;
	selPntTest = 0;
	selmode = SelectPoint;
	density = 1.0;
	autoDeselect = true;
	multithreaded = true;
	scope = 0;
	scopeIsScene = true;
	scopeInstance = 0;

	execSceneScope = 0;

	OperationStack::addStateHandler( &h );
}
//
void GlobalState::readState( const pt::datatree::Branch* b )
{
	int s;
	b->getNode("layers", layer.layersInt);
	b->getNode("autoDeselect", autoDeselect);
	b->getNode("selectionMode", s);
	selmode = (SelectionMode)s;

	b->getNode("editScope", scope);
	b->getNode("editScopeIsScene", scopeIsScene);
	b->getNode("editScopeInstance", scopeInstance);

    g_currentLayer = static_cast<ubyte>(layer.layers[0]);
	g_activeLayers = static_cast<ubyte>(layer.layers[1]);
	g_visibleLayers = static_cast<ubyte>(layer.layers[2]);
	g_lockedLayers = static_cast<ubyte>(layer.layers[3]);

	/* constraint state */ 
	datatree::Branch* c = b->getBranch("constraint");

	constraint = 0;
	selPntTest = 0;

	if (c)
	{
		String cname;
		if (c->getNode("name", cname ))
		{
			EditNodeDef *cnode = EditNodeDef::findNodeDef( cname.c_str() );
			if (cnode) 
			{
				constraint = (EditConstraint*)cnode;
				constraint->readState(c);
				selPntTest = constraint->constraintFunc();
			}
		}
	}

	pointsengine::ClipManager::instance().readState(b);
}
//
void GlobalState::writeState( pt::datatree::Branch* b )
{
	layer.layers[0] = g_currentLayer;
	layer.layers[1] = g_activeLayers;
	layer.layers[2] = g_visibleLayers;
	layer.layers[3] = g_lockedLayers;

	b->addNode("layers", layer.layersInt);
	b->addNode("autoDeselect", autoDeselect);
	b->addNode("selectionMode", (int)selmode);

	b->addNode("editScope", scope);
	b->addNode("editScopeIsScene", scopeIsScene);
	b->addNode("editScopeInstance", scopeInstance);

	/* constraint state */ 
	if (constraint)
	{
		datatree::Branch *c = b->addBranch("constraint");
		c->addNode("name", constraint->name());
		constraint->writeState(b);
	}

	pointsengine::ClipManager::instance().writeState(b);
}
PreserveState::PreserveState() : save("state")
{
	OperationStack * edit = PointEditManager::instance()->getCurrentEdit();
	g_state.writeState( &save );
	edit->writeStateBranch( &save );
}
PreserveState::~PreserveState()
{
	OperationStack * edit = PointEditManager::instance()->getCurrentEdit();

	g_state.readState( &save );
	edit->readStateBranch( &save );
}