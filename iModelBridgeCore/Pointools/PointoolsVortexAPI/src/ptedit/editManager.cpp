//-----------------------------------------------------------------------------------------------
//
// PT_EDIT Point cloud editing plugin
//
//-----------------------------------------------------------------------------------------------
#pragma warning (disable : 4786 )

#include "PointoolsVortexAPIInternal.h"
#include <wildmagic/math/Wm5matrix3.h>
#include <wildmagic/math/Wm5ApprPlaneFit3.h>

#include <pt/trace.h>

#include <ptedit/EditManager.h>
#include <ptedit/constriants.h>
#include <ptedit/editStackState.h>
#include <ptedit/cloudSelect.h>

#include <ptengine/renderengine.h>
#include <ptengine/pointLayers.h>
#include <ptengine/PointsFilter.h>
#include <ptengine/engine.h>

#include <fastdelegate/fastdelegate.h>
#include <ptl/dispatcher.h>


#include <stack>
#include <map>
#include <string>


using namespace pt;
using namespace ptl;
using namespace ptedit;
using namespace pcloud;
using namespace pointsengine;

#define HIDDEN_LAYER_MASK 0x80

namespace ptedit
{
	struct SelectionFilters
	{
		FilterOpClearAll				clearAll;
		FilterOpSelectAll				selectAll;
		FilterOpDeselectAll				deselectAll;
		FilterResetSelection			resetSelection;
		FilterOpHideSelected			hideSelected;
		FilterOpShowAll					showAll;
		FilterOpCopyToLayer				copyToLayer;
		FilterOpMoveToLayer				moveToLayer;
		FilterOpInvertSelection			invertSelection;
		FilterOpInvertVisibility		invertVisibility;
		FilterOpIsolateSelected			isolateSelected;
		FilterOpConsolidate				consolidate;
		FilterOpConsolidateAllLayers	consolidateAllLayers;
		FilterOpSetLayer				setLayer;
		FilterOpDeselectLayer			deselectLayer;
		FilterOpSelectLayer				selectLayer;

		CloudSelect						cloudselect;
	};
	static SelectionFilters	*s_filters=0;
}

/*****************************************************************************/
/**
* @brief	PointEditManager constructor
*/
/*****************************************************************************/
PointEditManager::PointEditManager()
{
	omp_set_num_threads(4);
	m_units = 1.0;

	g_editWorkingMode = EditWorkOnView;

	s_filters = new SelectionFilters;
}

/*****************************************************************************/
/**
* @brief						singleton instance
* @return PointEditManager *
*/
/*****************************************************************************/
PointEditManager * PointEditManager::instance()
{
	static PointEditManager *_instance = 0;
	if (!_instance) _instance = new PointEditManager();
	return _instance;
}
/*****************************************************************************/
/**
* @brief
* @return OperationStack*
*/
/*****************************************************************************/
OperationStack* PointEditManager::getCurrentEdit()
{
	return &m_currentEdit;
}
/*****************************************************************************/
/**
* @brief
* @param name
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::storeEdit( const pt::String name )
{
	if (m_edits.find(name) != m_edits.end()) return false;

	OperationStack *edit = new OperationStack( m_currentEdit.stack() );

	m_edits.insert(EditMap::value_type(name, edit));
	return true; 
}

/*****************************************************************************/
/**
* @brief
* @param name
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::restoreEdit( const pt::String name )
{
	EditMap::iterator i = m_edits.find(name);
	if (i!= m_edits.end())
	{
		clearEdit();
		OperationStack *edit = (OperationStack*)(i->second);
		m_currentEdit.readStack( edit->stack() );
		m_currentEdit.execute();
		return true;
	}
	else return false;
}

/*****************************************************************************/
/**
* @brief
* @param name
* @return void
*/
/*****************************************************************************/
void PointEditManager::removeEdit (const pt::String name )
{
    std::lock_guard<std::mutex> lock(m_processMutex);

	EditMap::iterator i = m_edits.find(name);
	if (i!= m_edits.end())
	{
		delete i->second;
		m_edits.erase(i);
	}
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::removeAllEdits ()
{
    std::lock_guard<std::mutex> lock(m_processMutex);

	EditMap::iterator i = m_edits.begin();
	while (i!=m_edits.end())
	{	
		delete i->second;
		++i;
	}
	m_edits.clear();
}

/*****************************************************************************/
/**
* @brief
* @param use
* @return void
*/
/*****************************************************************************/
void PointEditManager::setUseMultiThreading( bool use ) 
{ 
	g_state.multithreaded = use; 
}


/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::getUseMultiThreading() const 
{ 
	return g_state.multithreaded; 
}
/*****************************************************************************/
/**
* @brief
* @return ptedit::SelectionMode
*/
/*****************************************************************************/
SelectionMode PointEditManager::editMode() const
{
	return g_state.selmode;
}
/*****************************************************************************/
/**
* @brief
* @param mode
* @return void
*/
/*****************************************************************************/
void			PointEditManager::workingMode( EditWorkingMode mode )
{
	g_editWorkingMode = (uint)mode;
}
/*****************************************************************************/
/**
* @brief
* @return ptedit::EditWorkingMode
*/
/*****************************************************************************/
EditWorkingMode	PointEditManager::workingMode() const
{
	return (EditWorkingMode)g_editWorkingMode;
}
/*****************************************************************************/
/**
* @brief
* @return int
*/
/*****************************************************************************/
int		PointEditManager::stateId() const
{
	return m_currentEdit.stateId();
}

/*****************************************************************************/
/**
* @brief
* @return uint
*/
/*****************************************************************************/
uint PointEditManager::numEdits() const
{
	return m_edits.size();
}

/*****************************************************************************/
/**
* @brief
* @param index
* @return const pt::String &
*/
/*****************************************************************************/
const pt::String &PointEditManager::editName( uint index )
{
	EditMap::iterator i = m_edits.begin();
	for (uint id=0; (id<index && i!=m_edits.end()); id++)
	{
		++i;
	}
	if (i!=m_edits.end()) return i->first;
	static pt::String undef("undefined");
	return undef;
}
/*****************************************************************************/
/**
* @brief
* @param unitsPerMetre
* @return void
*/
/*****************************************************************************/
void PointEditManager::setUnits( double unitsPerMetre )
{
	if (unitsPerMetre > 0 && unitsPerMetre < 1e6)
		m_units = unitsPerMetre;
}
/*****************************************************************************/
/**
* @brief
* @return double
*/
/*****************************************************************************/
double PointEditManager::getUnits() const
{
	return m_units;
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::selectMode()	
{ 
	g_state.selmode = SelectPoint; 
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::deselectMode()	
{ 
	PTTRACE_FUNC

	g_state.selmode = DeselectPoint; 
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::unhideMode()	
{ 
	g_state.selmode = UnhidePoint; 
}

/*****************************************************************************/
/**
* @brief
* @param guid
* @param sceneScope
* @return void
*/
/*****************************************************************************/
void PointEditManager::setEditingScope( pcloud::PointCloudGUID guid, bool sceneScope, int sceneInstance )
{
	g_state.scope = guid;
	g_state.scopeIsScene = sceneScope;
	g_state.scopeInstance = sceneInstance;
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::clearEditingScope()
{
	g_state.scope = 0;
	g_state.scopeIsScene = false;
	g_state.scopeInstance = 0;
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::updateActiveLayers()
{
	g_activeLayers = g_visibleLayers;
	g_activeLayers &= ~g_lockedLayers;

	pointsengine::thePointLayersState().setCurrentBitMask( (uint)g_currentLayer );
	pointsengine::thePointLayersState().setVisibleBitMask( (uint)g_visibleLayers );
}

/*****************************************************************************/
/**
* @brief		Performs the task of updating the node level layer states
* @return void
*/
/*****************************************************************************/
void PointEditManager::updateLayerVisibility()
{
	EditNodeDef::applyByName( "SetLayer" );

	pointsengine::thePointLayersState().setCurrentBitMask( (uint)g_currentLayer );
	pointsengine::thePointLayersState().setVisibleBitMask( (uint)g_visibleLayers );
}
/*****************************************************************************/
/**
* @brief		Calculates the approximate layer bounding gboc
* @return void
*/
/*****************************************************************************/
bool	PointEditManager::getVisibleLayersBoundingBox( pt::BoundingBoxD &bb, bool fast_approx ) const
{
	ComputeLayerBoundsVisitor boundscompute( g_visibleLayers, fast_approx );
	TraverseScene::withVisitor(&boundscompute);

	bb = boundscompute.boundingBox();

	return bb.isEmpty() ? false : true;
}
/*****************************************************************************/
/**
* @brief		Performs the task of updating the node level layer states
* @return void
*/
/*****************************************************************************/
bool	PointEditManager::getLayerBoundingBox( int layerIndex, pt::BoundingBoxD &bb, bool fast_approx ) const 
{
	if (layerIndex < 0) return false;

	ubyte layerMask = 1 << layerIndex;

	ComputeLayerBoundsVisitor boundscompute( layerMask, fast_approx );
	TraverseScene::withVisitor(&boundscompute);

	bb = boundscompute.boundingBox();

	return bb.isEmpty() ? false : true;
}
/*****************************************************************************/
/**
* @brief		Performs the task of updating the node level layer states
* @return void
*/
/*****************************************************************************/
bool	PointEditManager::doesLayerHavePoints( int layerIndex ) const
{
	if (layerIndex < 0) return false;

	ubyte layerMask = 1 << layerIndex;

	// first do fast check
	DoesLayerHavePointsVisitor checkForPoints( layerMask, true );
	TraverseScene::withVisitor(&checkForPoints);

	if (checkForPoints.hasPoints()) return true;
	
	// then slow
	checkForPoints.approx( false );
	TraverseScene::withVisitor(&checkForPoints);

	return checkForPoints.hasPoints();
}
/*****************************************************************************/
/**
* @brief
* @param layer
* @param lock
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::lockLayer( int layer, bool lock )
{
	if (lock)
		g_lockedLayers |= (1<<layer);
	else g_lockedLayers &= ~(1<<layer);
	
	updateActiveLayers();

	return true;
}
/*****************************************************************************/
/**
* @brief
* @param layer
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::isLayerVisible( int layer ) const
{
	if (g_visibleLayers & (1 << layer)) return true;
	return false;
}
/*****************************************************************************/
/**
* @brief
* @param layer
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::isLayerLocked( int layer ) const
{
	if (g_lockedLayers & (1 << layer)) return true;
	return false;
}
/*****************************************************************************/
/**
* @brief
* @param layer
* @param show
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::showLayer( int layer, bool show )
{
	PTTRACE_FUNC

	ubyte layerMask = (1 << layer);

	if ( layerMask == g_currentLayer )
		return show;
	
	if (show)	g_visibleLayers |= layerMask;
	else		g_visibleLayers &= ~layerMask;

	updateActiveLayers();
	updateLayerVisibility();

	return true;
}
/*****************************************************************************/
/**
* @brief
* @param layer
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::setCurrentLayer( int layer, bool maskValue )
{
	PTTRACE_FUNC

	ubyte blyr = maskValue ? layer : (1 << layer);

	if (g_lockedLayers & blyr) return false;

	g_visibleLayers |= blyr;
	g_currentLayer = blyr;
	g_activeLayers = g_visibleLayers & ~g_lockedLayers;

	//EditNodeDef::applyByName( "Consolidate" );

	pointsengine::thePointLayersState().setCurrentBitMask( (uint)g_currentLayer );
	pointsengine::thePointLayersState().setVisibleBitMask( (uint)g_visibleLayers );

	updateLayerVisibility();

	return true;
}
/*****************************************************************************/
/**
* @brief
* @return int
*/
/*****************************************************************************/
int PointEditManager::getCurrentLayer() const
{
	switch (g_currentLayer)
	{
	case 0: return 8; /* hidden layer */ 
	case 1: return 0;
	case 2: return 1;
	case 4: return 2;
	case 8: return 3;
	case 16: return 4;
	case 32: return 5;
	case 64: return 6;
	case 128: return 7;
	default: return 0;
	}
}


/*****************************************************************************/
/**
* @brief
* @param deselect
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::moveSelToLayer( bool deselect )
{
	PTTRACE_FUNC

	if (g_activeLayers & HIDDEN_LAYER_MASK)
	{
		/* move to hidden layer */ 
		hideSelPoints();
	}
	else
	{
		m_currentEdit.addOperation( "MoveToLayer" );	
		setCurrentLayer( getCurrentLayer() );
	}
	return true;
}
//
/*****************************************************************************/
/**
* @brief
* @param deselect
* @return bool
*/
/*****************************************************************************/
bool PointEditManager::copySelToLayer( bool deselect )
{
	PTTRACE_FUNC

	if (g_activeLayers & HIDDEN_LAYER_MASK) return false;

	m_currentEdit.addOperation( "CopyToLayer" );
	setCurrentLayer( getCurrentLayer() );

	return true;
}
//
bool	PointEditManager::selectPointsInLayer( int layer )
{
	PTTRACE_FUNC

	s_filters->selectLayer.targetLayer(layer);
	m_currentEdit.addOperation( &s_filters->selectLayer );

	return true;
}
//
bool	PointEditManager::deselectPointsInLayer( int layer )
{
	PTTRACE_FUNC

	s_filters->deselectLayer.targetLayer(layer);
	m_currentEdit.addOperation( &s_filters->deselectLayer );

	return true;
}
//
// These cloud and scene selection funcs are not exposed 
// since scope can be used to the same effect
//
bool	PointEditManager::selectPointcloud( pcloud::PointCloud *cloud )
{
	PTTRACE_FUNC

	s_filters->cloudselect.setCloud( cloud );
	m_currentEdit.addOperation( &s_filters->cloudselect );

	return true;
}
bool	PointEditManager::selectScene( pcloud::Scene *scene )
{
	// not implemented
	return false;
}

//-----------------------------------------------------------------------------
// stores the state of all nodes in the point clouds - must be called for writing
// in EXACTLY same order as read ie. only within same function
//-----------------------------------------------------------------------------
class StoreNodeStateAndPrepForRefresh : public pcloud::Node::Visitor
{
	public:
		StoreNodeStateAndPrepForRefresh()	
		{			
			_readMode = true;
		}
		void writeMode() { _readMode = false; _pos = 0; }

		bool visitNode( const pcloud::Node *node )
		{
			Node *mnode = const_cast<pcloud::Node*>(node);

			if (_readMode)
			{
				NodeState ns;
				ns.layers0 = node->layers(0);
				ns.layers1 = node->layers(1);
				ns.wholeSel = node->flag(pcloud::WholeSelected);
				ns.partSel = node->flag(pcloud::PartSelected);

				_nodes.push_back(ns);

				if (node->layers(1) || node->flag( pcloud::PartSelected ))
				{
					mnode->flag( pcloud::Flagged, true, false );
				}
			}
			else
			{
				mnode->layers(0) = _nodes[_pos].layers0;
				mnode->layers(1) = _nodes[_pos].layers1;
				mnode->flag(pcloud::WholeSelected, _nodes[_pos].wholeSel);
				mnode->flag(pcloud::PartSelected, _nodes[_pos].partSel);
				_pos++;

				mnode->flag( pcloud::Flagged, false, false ); // clean up
			}
			return true;
		}

		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) {}
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}

		struct NodeState
		{
			ubyte layers0;
			ubyte layers1;
			bool  wholeSel;
			bool  partSel;
		};

		int						_pos;
		pcloud::Voxel			*_currentVoxel;
		bool					_readMode;
		std::vector<NodeState>	_nodes;
};


//-----------------------------------------------------------------------------
// sets the number of points edited to the current LOD
// this is needed for a refresh run of the stack because a refresh
// only processes the points from the last number of points edited to the
// current LOD or end (if OOC)
//-----------------------------------------------------------------------------
class SetEditPointToLODVisitor : public pcloud::Node::Visitor
{
	public:
		SetEditPointToLODVisitor() {}

		bool visitNode( const pcloud::Node *node )
		{
			if (node->isLeaf())
			{
				pcloud::Voxel *v = 
					const_cast<pcloud::Voxel*>( static_cast<const pcloud::Voxel*>(node) );
				v->numPointsEdited( v->lodPointCount() * g_state.density );

				if (g_editApplyMode & EditIncludeOOC)
				{
					v->numPointsEdited( v->fullPointCount() );
				}
			}
			return true;
		}

		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) {}
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}
};

void	PointEditManager::updateSceneEditStateID( pcloud::Scene *scene )
{
	if (scene)
	{
		scene->setEditStateID( m_currentEdit.stateId() );
	}
	else
	{
		for (int i=0; i<thePointsScene().size();i++)
		{
			pcloud::Scene* scene = pointsengine::thePointsScene()[i];
			scene->setEditStateID( m_currentEdit.stateId() );
		}	
	}
}


void PointEditManager::regenEditQuick_run()
{
	PTTRACE_FUNC

	g_editApplyMode = EditNormal;

	// scope should not effect this, will be restored by PreserveState
	g_state.scope = 0;

	// flag nodes first - note this cannot be done any other way since node state will be changing
	StoreNodeStateAndPrepForRefresh statev;
	TraverseScene::withVisitor(&statev);

	g_editApplyMode = EditIntentRefresh | EditIntentFlagged;
	m_currentEdit.execute(false);

	g_editApplyMode = EditNormal;

	// need to set the edit points in voxels to current lod 
	// because this is not done in refresh mode
	SetEditPointToLODVisitor sep;
	TraverseScene::withVisitor(&sep);

	statev.writeMode();
	TraverseScene::withVisitor(&statev);

	//ClearFilterVisitor v;
	//TraverseScene::withVisitor(&v);

	EditNodeDef::applyByName( "ConsolidateAllLayers" );	
	
	setCurrentLayer( g_currentLayer, true );
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::regenEditQuick()
{
	PTTRACE_FUNC

	{
		PreserveState save;

		pointsengine::pauseEngine();
	
		// this is experimental!
		// boost::thread regen( MakeDelegate(this, &PointEditManager::regenEditQuick_run) );

		regenEditQuick_run();

		pointsengine::unpauseEngine();

	}
	return;
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::regenEditComplete() // NOT TESTED
{
	PTTRACE_FUNC

	PreserveState save;

	pointsengine::pauseEngine();

	g_editApplyMode = EditNormal;

	// scope should not effect this, will be restored by PreserveState
	g_state.scope = 0;

	// flag nodes first - note this cannot be done any other way since node state will be changing
	StoreNodeStateAndPrepForRefresh statev;
	TraverseScene::withVisitor(&statev);

	g_editApplyMode =  EditIntentFlagged;
	m_currentEdit.execute(false);

	g_editApplyMode = EditNormal;


	statev.writeMode();
	TraverseScene::withVisitor(&statev);

	//ClearFilterVisitor v;
	//TraverseScene::withVisitor(&v);

	EditNodeDef::applyByName( "ConsolidateAllLayers" );	
	
	setCurrentLayer( g_currentLayer, true );

	pointsengine::unpauseEngine();
}
//-----------------------------------------------------------------------------
//  Only regen those scenes that have not been processed at all yet
//-----------------------------------------------------------------------------
void	PointEditManager::regenEditUnprocessed()
{
	// check the state of every scene and process if needed
	for (int i=0;i<thePointsScene().size();i++)
	{
		pcloud::Scene *scene = thePointsScene()[i];

		if (scene->editStateID() <=0 )
		{
			regenOOCComplete( scene );
		}
	}
}
//-----------------------------------------------------------------------------
// complete regen from scratch - not optimised
//-----------------------------------------------------------------------------
void PointEditManager::regenOOCComplete( pcloud::Scene *scene )
{
	PTTRACE_FUNC

	PreserveState save;

	pointsengine::pauseEngine();

	g_editApplyMode = EditNormal;

	// scope should not effect this, will be restored by PreserveState
	g_state.scope = 0;
	g_state.setExecutionScope( scene );

	// flag nodes first - note this cannot be done any other way since node state will be changing
	StoreNodeStateAndPrepForRefresh statev;
	
	if ( scene )
	{
		TraverseScene::withVisitor(&statev, scene, false);
	}
	else
	{
		TraverseScene::withVisitor(&statev, false);
	}
	g_editApplyMode =  EditIntentRefresh | EditIntentFlagged | EditIncludeOOC;
	m_currentEdit.execute(false);

	g_editApplyMode = EditNormal | EditIncludeOOC;

	// this is ok here, has a hack to set to full if EditIncludeOOC
	SetEditPointToLODVisitor sep;

	if (scene)
	{
		TraverseScene::withVisitor(&sep, scene, false);
	}
	else 
	{
		TraverseScene::withVisitor(&sep, false);
	}
	
	g_editApplyMode = EditNormal;

	statev.writeMode();

	if (scene)
	{
		TraverseScene::withVisitor(&statev, scene, false);
	}
	else
	{
		TraverseScene::withVisitor(&statev, false);
	}

	//ClearFilterVisitor v;
	//TraverseScene::withVisitor(&v);

	EditNodeDef::applyByName( "ConsolidateAllLayers" );	
	
	setCurrentLayer( g_currentLayer, true );

	updateSceneEditStateID( scene );

	g_state.setExecutionScope( 0 );

	pointsengine::unpauseEngine();
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::clearEdit()
{
	PTTRACE_FUNC

	{
		PreserveState save;
		uint saveApplyMode = g_editApplyMode;
		g_editApplyMode = 0;

		//clearColourEdits();

		g_editApplyMode = 0;
		m_currentEdit.clear();
		clearAll();

		g_editApplyMode = saveApplyMode;
	}
	setCurrentLayer( g_currentLayer, true );
}


/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
PaintBrush *_activeBrush = &g_selBrush.brush;

void PointEditManager::paintSelSphere()
{
	_activeBrush = &g_selBrush.brush;
	_activeBrush->shape( BrushShapeBall );

	g_paint.brush.shape(BrushShapeBall);
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::paintSelCube()
{
	_activeBrush = &g_selBrush.brush;
	_activeBrush->shape( BrushShapeBox );
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::selectAll()
{
	PTTRACE_FUNC

	m_currentEdit.addOperation( "SelectAll" );
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::deselectAll()
{
	PTTRACE_FUNC

	m_currentEdit.addOperation( "DeselectAll" );
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::resetSelection()
{
	PTTRACE_FUNC

	m_currentEdit.addOperation( "ResetSelection" );
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::clearAll()
{
	PTTRACE_FUNC

	/* old implementation (bugged) left for testing
	ClearFilterVisitor c;
	TraverseScene::withVisitor( &c );
	*/
	m_currentEdit.addOperation( "ClearAll" );
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::showAll()
{
	PTTRACE_FUNC

	m_currentEdit.addOperation( "ShowAll" );
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::invertSelection()
{	
	PTTRACE_FUNC

	m_currentEdit.addOperation( "InvertSel" );
}
/*****************************************************************************/
/**
* @brief
* @return __int64
*/
/*****************************************************************************/
__int64 PointEditManager::countVisiblePoints()
{
	PTTRACE_FUNC

	CountVisibleVisitor v;
	TraverseScene::withVisitor( &v, true );

	return v.totalCount();
}
/*****************************************************************************/
/**
* @brief
* @return __int64
*/
/*****************************************************************************/
__int64 PointEditManager::countPointsInLayer( int layerIndex )
{
	PTTRACE_FUNC

	if (layerIndex < 0) return 0;
	ubyte layerMask = 1 << layerIndex;

	CountPointsInLayerVisitor v(layerMask);
	TraverseScene::withVisitor( &v, true );

	return v.totalCount();
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::invertVisibility()
{	
	PTTRACE_FUNC

	m_currentEdit.addOperation( "InvertVis" );
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::isolateSelPoints()
{
	PTTRACE_FUNC

	m_currentEdit.addOperation( "IsolateSelected" );
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::hideSelPoints()
{
	PTTRACE_FUNC

	m_currentEdit.addOperation( "HideSelected" );
}


/*****************************************************************************/
/**
* @brief
* @param radius
* @return void
*/
/*****************************************************************************/
void PointEditManager::paintRadius( double radius )
{
	_activeBrush->radius( radius );

	g_paint.brush.radius(radius);
}

/*****************************************************************************/
/**
* @brief
* @return float
*/
/*****************************************************************************/
float PointEditManager::getPaintRadius( )
{
	return _activeBrush->radius();
}
/*****************************************************************************/
/**
* @brief
* @param params
* @return void
*/
/*****************************************************************************/
void PointEditManager::setViewParams( const pt::ViewParams &params )
{
	m_view = params;
	m_view.updatePipeline();
}

/*****************************************************************************/
/**
* @brief
* @param pnt
* @param limit_range
* @return void
*/
/*****************************************************************************/
void PointEditManager::paintSelectAtPoint( const pt::vector3d &pnt, bool limit_range )
{
	static pt::vector3d lastpoint(0,0,0);

	pt::vector3d cam;

	///* check for depth jump */ 
	float ddepth = lastpoint.dist(pnt);//fabs(cam.dist(pnt) - cam.dist(lastpoint));
	
	if ( !limit_range || lastpoint.is_zero() || ddepth  <  _activeBrush->radius() * 3)
	{
		m_paintSelect.setCenter( pnt );
		m_currentEdit.addOperation( &m_paintSelect, true, true );
		lastpoint = pnt;
	}
}

/*****************************************************************************/
/**
* @brief
* @param l
* @param r
* @param b
* @param t
* @return void
*/
/*****************************************************************************/
void PointEditManager::rectangleSelect( int l, int r, int b, int t)
{
	PTTRACE_FUNC

	Recti rect(l, t, r, b);
	rect.makeValid();

	m_frustumSelect.buildFromScreenRect(rect, m_view, m_units) ;//these parameters also needeed:, m_view, m_units);
	m_currentEdit.addOperation(&m_frustumSelect);
}

/*****************************************************************************/
/**
* @brief
* @param lower
* @param upper
* @return void
*/
/*****************************************************************************/
void PointEditManager::boxSelect( const pt::vector3d &lower, const pt::vector3d &upper )
{
	PTTRACE_FUNC

	m_boxSelect.set(lower, upper);

	m_currentEdit.addOperation(&m_boxSelect);
}
/*****************************************************************************/
/**
* @brief
* @param lower
* @param upper
* @param position
* @param uAxis
* @param vAxis
* @return void
*/
/*****************************************************************************/
void PointEditManager::orientedBoxSelect( const pt::vector3d &lower, const pt::vector3d &upper, const pt::vector3d &pos, const pt::vector3d &uAxis, const pt::vector3d &vAxis)
{
	PTTRACE_FUNC

	m_orientedBoxSelect.set(lower, upper);
	m_orientedBoxSelect.setTransform(pos, uAxis, vAxis);

	m_currentEdit.addOperation(&m_orientedBoxSelect);

	//s_orientedBoxSelFilter.filterData.box.set(lower, upper);
	//s_orientedBoxSelFilter.filterData.setTransform(position, uAxis, vAxis);
	//m_currentEdit.addEditOperation( s_orientedBoxSelFilter );
	//s_lastTime = s_frustumSelFilter.lastProcessTimeInMs();
}

/*****************************************************************************/
/**
* @brief
* @param origin
* @param normal
* @param thickness
* @return void
*/
/*****************************************************************************/
void PointEditManager::planeSelect( const pt::vector3d &origin, const pt::vector3d &normal, double thickness )
{
	PTTRACE_FUNC

	vector3d od( origin );
	vector3d nd( normal );


	//s_planeSelect.
	m_planeSelect.setPlane( vector3d(origin), vector3d(normal) );
	m_planeSelect.thickness = thickness;
	m_planeSelect.unbounded = true;

	m_currentEdit.addOperation( &m_planeSelect );
}
/*****************************************************************************/
/**
* @brief
* @param fence
* @return void
*/
/*****************************************************************************/
void PointEditManager::fenceSelect( const pt::Fence<int> &fence )
{
	PTTRACE_FUNC

	m_fenceSelect.buildFromScreenFence(fence, m_view, m_units);
	m_currentEdit.addOperation(&m_fenceSelect);
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointEditManager::layersFromUserChannel( pointsengine::UserChannel* userChannel )
{
	PTTRACE_FUNC

	if (userChannel)
	{
		m_layersFromUserChannel.setUserChannel(userChannel);
		m_currentEdit.addOperation("LayersFromUserChannel");

		updateSceneEditStateID( 0 );
	}
}
/*****************************************************************************/
/**
* @brief
* @param index
* @return pt::datatree::Branch *
*/
/*****************************************************************************/
pt::datatree::Branch *PointEditManager::_getEditDatatree( int index )
{
	// not supporting multiple edits
	if (index == 0 && m_edits.size() == 0)
	{
		datatree::Branch *root = const_cast<datatree::Branch*>(m_currentEdit.stack());
		return root;
	}

	int c=0;
	EditMap::iterator i = m_edits.begin();

	while (c++ < index) ++i;

	if (i!= m_edits.end())
	{
		OperationStack *edit = (OperationStack*)(i->second);
		datatree::Branch *root = const_cast<datatree::Branch*>(edit->stack());
		
		root->addNode( "name", i->first );
		return root;
	}
	return 0;
}
/*****************************************************************************/
/**
* @brief
* @param dtree
* @return void
*/
/*****************************************************************************/
void PointEditManager::_createEditFromDatatree( pt::datatree::Branch * dtree )
{
	OperationStack *edit = new OperationStack(dtree);
	pt::String name;
	if (dtree->getNode( "name", name))
		m_edits.insert(EditMap::value_type(name, edit));
	else	// this is intended to replace the current edit stack
	{
		m_currentEdit.readStack( dtree );
		m_currentEdit.execute();
	}
}
