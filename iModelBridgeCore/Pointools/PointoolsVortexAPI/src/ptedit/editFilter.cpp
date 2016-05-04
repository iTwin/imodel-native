//-----------------------------------------------------------------------------------------------
//
// PT_EDIT Point cloud editing plugin
//
//-----------------------------------------------------------------------------------------------
#pragma warning (disable : 4786 )

#include <Wm5matrix3.h>
#include <Wm5ApprPlaneFit3.h>


#include <pt/trace.h>
#include <ptengine/engine.h>
#include <ptedit/selectionFilter.h>
#include <ptengine/renderengine.h>
#include <pttool/tool.h>

#include <ptedit/paintfilters.h>
#include <ptedit/fencefilter.h>
#include <ptedit/frustumfilter.h>
#include <ptedit/boxfilter.h>
#include <ptedit/planefilter.h>
#include <ptedit/filterops.h>

#include <ptgl/gltext.h>
#include <ptgl/gldraw.h>

#include <fastdelegate/fastdelegate.h>

#include <ptl/dispatcher.h>
#include <map>
#include <string>

#define HIDDEN_LAYER_MASK 0x40

using namespace pt;
using namespace ptl;
using namespace ptedit;
using namespace ptapp;
using namespace pcloud;
using namespace pointsengine;

namespace ptedit {
namespace detail
{
	SelectionMode g_mode = SelectPoint;
	static SelectionFilter<PaintSelectFilter>	s_paintSelFilter;
	static SelectionFilter<FrustumFilter>		s_frustumSelFilter;
	static SelectionFilter<BoxFilter>			s_boxSelFilter;
	static SelectionFilter<OrientedBoxFilter>	s_orientedBoxSelFilter;
	static SelectionFilter<FenceFilter>			s_fenceSelFilter;
	static SelectionFilter<PlanarPolyFilter>	s_planeSelFilter;
	static SelectionFilter<PaintFilter>			s_paintFilter;

	static FilterOpSetLayer			s_opSetLayer;
	static FilterOpCopyToLayer		s_opCopyToLayer;
	static FilterOpMoveToLayer		s_opMoveToLayer;
	static FilterOpShowAll			s_opShowAll;
	static FilterOpDeselectAll		s_opDeselectAll;
	static FilterOpSelectAll		s_opSelectAll;
	static FilterOpHideSelected		s_opHideSelected;
	static FilterOpInvertSelection	s_opInvertSel;
	static FilterOpInvertVisibility s_opInvertVis;
	static FilterOpConsolidate		s_opConsolidate;
	static FilterOpPaintErase		s_opPaintErase;

	int		s_lastTime = 0;
	int		s_numOperations = 0;
	String _layerNames[32];
	
	ubyte	g_currentLayer = 1;
	ubyte	g_currentLayerP = 2;

	ubyte	g_activeLayers = 1;
	ubyte	g_activeLayersP = 2;
	
	ubyte	g_visibleLayers = 1;
	ubyte	g_visibleLayersP = 2;

	ubyte	g_lockedLayers = 0;
	ubyte	g_lockedLayersP = 0;

	ubyte	_colBuffer[] = { 128,128,128 };  

	/* state */ 
	bool			g_autoDeselect = true;			// deselect after move or copy
	ubyte			*g_paintCol = _colBuffer;		// current paint colour
	float			g_paintAlpha = 1.0f;			// current paint alpha
	int				g_paintMode = PaintModeAdd;		// paint blending mode
	pcloud::Node	*g_voxelOnly = 0;				// limit editing to this voxel - suspect unused now
	float			g_editDensity = 1.0f;			// density at which editing is processed
	EditApplyMode	g_editApplyMode = EditNormal;	// mode to use when editing is apply (executed from history)

	PointTestFunc	g_selPntTest = 0;				// per point test, used to implement constraints

	bool			g_multithreaded = false;		// use multiple threads
	
	PointCloudGUID	g_scope = 0;					// scope selections
	bool			g_scopeIsScene = false;			// scope GUID is for parent scene object, not cloud only

	static BranchHandler		*_branchHandler;
	static pt::ParameterMap		_params;
	static std::vector<vector3> _planePoints;

	/* filters */ 
	typedef FastDelegate0<>									ProcessFilterCB;
	typedef FastDelegate1<const datatree::Branch *>			LoadStateCB;
	typedef FastDelegate2<pcloud::Voxel*, SelectionMode>	ProcessVoxelCB;
	typedef FastDelegate1<uint, bool>						InfoFlagCB;

	boost::mutex _processMutex;

	struct FilterCallbacks
	{
		FilterCallbacks(LoadStateCB load, ProcessFilterCB process, ProcessVoxelCB voxel, InfoFlagCB info)
			: loadState(load), processFilter(process), processVoxel(voxel), flag(info) {}

		template <class F>
		static FilterCallbacks make(F &filter)
		{
			FilterCallbacks fcb(
				MakeDelegate(&filter, &F::loadStateBranch),
				MakeDelegate(&filter, &F::processFilter),
				MakeDelegate(&filter, &F::processVoxel),
				MakeDelegate(&filter, &F::flag));
			
			return fcb;
		}
		ProcessFilterCB processFilter;
		LoadStateCB		loadState;
		ProcessVoxelCB	processVoxel;
		InfoFlagCB		flag;
	};

	typedef std::map<std::string, FilterCallbacks> FilterMap;
	static FilterMap s_filters;
};
};
using namespace ptedit::detail;

//
// pointLayerMask
//
static ubyte pointLayerMask( ubyte layerMask )
{
	ubyte pntLyr = (layerMask << 1);
	pntLyr &= ~1;
	pntLyr &= ~128;

	return pntLyr;
}
//
// PointEdit History
//
class PointEditHistory
{
public:

	PointEditHistory()
	{
		_operations = new datatree::Branch("Selection");
		_group = false;
		_subbranch =0;
	}
	PointEditHistory( pt::datatree::Branch *operations )
	{
		_operations = new datatree::Branch("Selection");
		*_operations = *operations;
		_group = false;
		_subbranch =0;

		RenumberOperationsVisitor r;
		_operations->visitBranches(r);
	}
	void operator = (const PointEditHistory &edit)  
	{
		this->clear();
		*_operations = *edit._operations;

		RenumberOperationsVisitor r;
		_operations->visitBranches(r);
	}

	/* updates visibility to the active layers */ 
	void updateLayerVisibility( pcloud::Voxel *vox=0 )
	{
		if (!vox) s_opSetLayer.processFilter();
		else s_opSetLayer.processVoxel(vox, g_mode);
	}
	void stepBack();
	void stepForward();

	void apply()
	{
		boost::mutex::scoped_lock lock( _processMutex );

		ubyte savelayers = g_activeLayers;
		//setLayer(1);
		SelectionMode savemode = g_mode;

		_vox = 0;
		_operations->visitBranches(*this);

		//setLayer(savelayers);
		g_mode = savemode;
	}
	void apply(pcloud::Voxel *voxel)
	{
		assert(0); /* should not be used */ 
	}
	/* visitor that applys the filter operation */ 
	void operator ()(datatree::Branch *b)
	{
		static bool inGroup = false;

		String id;
		datatree::Branch *fb = b;
		if (b->getBranch("group"))
		{
			inGroup = true;
			b->getBranch("group")->visitBranches(*this);
			inGroup = false;
			return;
		}
		if (!b->getNode("operation", id))
		{
			fb = b->getBranch(0);
			if (fb) id = fb->_id.get();
			else return; /* error */ 
		}
		restoreStateFromBranch(b);

		FilterMap::iterator i = s_filters.find(id.c_str());

		if (i!= s_filters.end())
		{
			i->second.loadState(fb);
		
			bool mt = g_multithreaded;
			if (!i->second.flag(MultithreadCompatible))
				g_multithreaded = false;
			
			i->second.processFilter();
			
			/* consolidate if needed */ 
			if (i->second.flag(PostFilterConsolidate) && !inGroup)
			{
				s_opConsolidate.processFilter();
				updateLayerVisibility();
			}
			
			g_multithreaded = mt;
		}
	}
	union Layers
	{
		Layers(ubyte current, ubyte active, ubyte locked, ubyte visible)
		{ layers[0] = current; layers[1] = active; layers[2] = locked; layers[3] = visible; }
		Layers(int64_t state) : layersInt(state) {}
		ushort layers[4];
		int64_t layersInt;
	};
	static int64_t layerState()
	{
		Layers layers(g_currentLayer, g_activeLayers, g_lockedLayers, g_visibleLayers);
		return layers.layersInt;		
	}
	void saveStateInBranch( datatree::Branch *b )
	{
		int64_t layers = layerState();

		b->addNode("mode", (int)g_mode);
		b->addNode("layers", layers);
		uint col = RGB(g_paintCol[0], g_paintCol[1], g_paintCol[2]);
		b->addNode("paintCol", col);
		b->addNode("paintAlpha", g_paintAlpha);
		b->addNode("paintMode", g_paintMode);
		b->addNode("scope", g_scope );
		b->addNode("scopeIsScene", g_scopeIsScene );
	}
	void restoreStateFromBranch( const datatree::Branch *b )
	{
		int64_t layers;
		int mode=1;

		if (b->getNode("layers", layers))
		{
			PointEdit::instance()->setLayerState( layers );
		}

		b->getNode("auto_deselect", g_autoDeselect);
		
		if (b->getNode("mode", mode))
			g_mode = (SelectionMode)mode;	

		uint col;
		if (b->getNode("paintCol", col))
		{
			g_paintCol[0] = GetRValue(col);
			g_paintCol[1] = GetGValue(col);
			g_paintCol[2] = GetBValue(col);
		}
		b->getNode("paintAlpha", g_paintAlpha);
		b->getNode("paintMode", g_paintMode);
		b->getNode("scope", g_scope );
		b->getNode("scopeIsScene", g_scopeIsScene );
	}
	template <class F>
	void addEditOperation(F &filter, bool apply=true, bool group=false)
	{
		boost::mutex::scoped_lock lock( _processMutex );

		char index[48];
		sprintf(index, "op %05d", s_numOperations++, filter.name());

		/* first item in group */ 
		if (group && !_group)
		{
			datatree::Branch *pb = _operations->addBranch(index);
			if (pb)
			{
				pb->addNode("desc", String(filter.filterData.desc()));
				pb->addNode("key", s_numOperations );
				pb->addNode("icon", filter.filterData.icon());
				saveStateInBranch(pb);
				_subbranch = pb->addBranch("group");
				++s_numOperations;
				_group = true;
			}
			else return; /* error */ 
		}
		else if (!group &&  _group)
		{
			_subbranch = 0;
			_group = false;
		}

		datatree::Branch *b = _subbranch ? _subbranch->addBranch(index) 
			: _operations->addBranch(index);

		b->addNode("desc", String(filter.filterData.desc()));
		b->addNode("key", s_numOperations );
		b->addNode("icon", filter.filterData.icon());
		if (!_group) saveStateInBranch(b);

		datatree::Branch *fb = b->addBranch(filter.name());
		
		filter.filterData.write(fb);

		/* apply this operation */ 
		if (apply)
		{
			(*this)(b);
		}
	}
	void closeGroup()
	{
		String desc;
		if (_subbranch)
		{
			_subbranch->_parent->getNode("desc", desc);
			char num[8];
			sprintf(num, " [%d]", _subbranch->numBranches());
			desc += pt::String(num);

			_subbranch->_parent->setNode("desc", String(desc));
			_subbranch = 0;

			/* all groups consolidate at the moment */ 
			s_opConsolidate.processFilter();
		}
		_group = false;
	}
	template <class F>
	void addGlobalOperation(F &filter, bool apply=true)
	{
		static bool lastAutoDeselect = true;

		char index[48];
		sprintf(index, "op %05d", s_numOperations++);
		String filterOp ( filter.name() );

		datatree::Branch *b = _operations->addBranch(index);
		b->addNode("key",  s_numOperations);
		b->addNode("operation", filterOp);
		b->addNode("desc", String(filter.desc()));
		b->addNode("icon", filter.icon());
		b->addNode("value", filter.value());

		saveStateInBranch(b);
		if (g_autoDeselect != lastAutoDeselect)
		{
			b->addNode("auto_deselect", g_autoDeselect);
			lastAutoDeselect = g_autoDeselect;
		}
		/* apply this operation */ 
		if (apply)
		{
			(*this)(b);
		}
	}
	template<class F>
	static void addFilter(F &filter)
	{
		s_filters.insert(FilterMap::value_type(filter.name(), FilterCallbacks::make(filter)));
	}

	void clear()
	{
		boost::mutex::scoped_lock lock( _processMutex );

		delete _operations;
		_operations = new datatree::Branch("Selection");
		s_numOperations = 0;
	}
	struct RenumberOperationsVisitor
	{
		RenumberOperationsVisitor()
		{
			s_numOperations = 0;
		}
		void operator ()(datatree::Branch *b)
		{
			b->setNode("key", s_numOperations);
			++s_numOperations;
		}
	};
	void readBranch(const datatree::Branch *b)
	{
		clear();

		datatree::Branch::CopyBranchVisitor v(_operations);
		const datatree::Branch *src = b->findBranch("Selection");
		if (src) src->visitNodes(v, true, false);

		RenumberOperationsVisitor r;
		_operations->visitBranches(r);
	}

	void removeBranch(datatree::Branch *b)
	{
		_operations->removeBranch(b->_id);
	}	
	const datatree::Branch *operationsBranch() { return _operations; }

protected:
	
	friend PointEdit;

	datatree::Branch *_operations;
	datatree::Branch *_subbranch;
	pcloud::Voxel *_vox;
	
	bool _group;
};
static PointEditHistory s_currentEdit;
/* --------------   management functions  -------------------------------*/ 
bool PointEdit::storeEdit( const pt::String name )
{
	if (_edits.find(name) != _edits.end()) return false;

	PointEditHistory *edit = new PointEditHistory();
	(*edit) = s_currentEdit;
	_edits.insert(EditMap::value_type(name, (void*)edit));

	return true;
}
//
bool PointEdit::restoreEdit( const pt::String name )
{
	EditMap::iterator i = _edits.find(name);
	if (i!= _edits.end())
	{
		clearEdit();
		s_currentEdit = (*(PointEditHistory*)(i->second));
		s_currentEdit.apply();
		return true;
	}
	else return false;
}
//
void PointEdit::removeEdit (const pt::String name )
{
	boost::mutex::scoped_lock lock( _processMutex );

	EditMap::iterator i = _edits.find(name);
	if (i!= _edits.end())
	{
		delete i->second;
		_edits.erase(i);
	}
}
//
void PointEdit::removeAllEdits ()
{
	boost::mutex::scoped_lock lock( _processMutex );

	EditMap::iterator i = _edits.begin();
	while (i!=_edits.end())
	{	
		delete i->second;
		++i;
	}
	_edits.clear();
}
// Multithreading
void PointEdit::setUseMultiThreading( bool use ) { g_multithreaded = use; }
bool PointEdit::getUseMultiThreading() const { return g_multithreaded; }
//
uint PointEdit::numEdits() const
{
	return _edits.size();
}
//
int64_t PointEdit::getLayerState() const
{
	PointEditHistory::Layers l(g_currentLayer, g_activeLayers, g_lockedLayers, g_visibleLayers);
	return l.layersInt;
}
//
void PointEdit::setLayerState(int64_t state)
{
	PointEditHistory::Layers l(state);
	
	g_currentLayer = l.layers[0];
	g_activeLayers = l.layers[1];
	g_lockedLayers = l.layers[2];
	g_visibleLayers = l.layers[3];

	g_currentLayerP = pointLayerMask( g_currentLayer );
	g_activeLayersP = pointLayerMask( g_activeLayers );
	g_visibleLayersP = pointLayerMask( g_visibleLayers );

	//s_currentEdit.updateLayerVisibility();
}
//
const pt::String &PointEdit::editName( uint index )
{
	EditMap::iterator i = _edits.begin();
	for (int id=0; (id<index && i!=_edits.end()); id++)
	{
		++i;
	}
	if (i!=_edits.end()) return i->first;
	static pt::String undef("undefined");
	return undef;
}
/*- end of management functions -----------------------------------------*/ 
void PointEdit::setUnits( double unitsPerMetre )
{
	if (unitsPerMetre > 0 && unitsPerMetre < 1e6)
		_units = unitsPerMetre;
}
double PointEdit::getUnits() const
{
	return _units;
}

void PointEdit::selectMode()	
{ 
	g_mode = SelectPoint; 
}
//
void PointEdit::deselectMode()	
{ 
	g_mode = DeselectPoint; 
}
//
void PointEdit::unhideMode()	
{ 
	g_mode = UnhidePoint; 
}
//
void PointEdit::setEditingScope( pcloud::PointCloudGUID guid, bool sceneScope )
{
	g_scope = guid;
	g_scopeIsScene = sceneScope;
}
void PointEdit::clearEditingScope()
{
	g_scope = 0;
	g_scopeIsScene = false;
}

//
static void outputLayerInfo()
{
	return;

	std::cout << "Current = " << (int)g_currentLayer << " / " << (int)g_currentLayerP << std::endl;
	std::cout << "Visible = " << (int)g_visibleLayers << " / " << (int)g_visibleLayersP << std::endl;
	std::cout << "Locked = " << (int)g_lockedLayers << " / " << (int)g_lockedLayersP << std::endl;
}
//
// Layer activeness (unlockedness)
//
bool PointEdit::lockLayer( int layer, bool lock )
{
	if ( (1 << layer) == g_currentLayer ) 
		return !lock;

	if (lock) g_lockedLayers |= (1 << layer);
	else g_lockedLayers &= ~(1 << layer);

	/* check current layer is not locked */ 
	g_activeLayers = g_visibleLayers & ~( g_lockedLayers );
	g_activeLayersP = pointLayerMask(g_activeLayers);

	outputLayerInfo();
	return true;
}
//
// layer visibility
//
bool PointEdit::showLayer( int layer, bool show )
{
	if ( (1 << layer) == g_currentLayer )
		return show;

	if (show) g_visibleLayers |= (1 << layer);
	else g_visibleLayers &= ~(1 << layer);

	g_visibleLayersP = pointLayerMask(g_visibleLayers);

	g_activeLayers = g_visibleLayers & ~g_lockedLayers;
	g_activeLayersP = pointLayerMask( g_activeLayers );

	s_currentEdit.updateLayerVisibility();

	outputLayerInfo();

	return true;
}
//
// current layer - can only be one layer used as target for Copy and Move
//
bool PointEdit::setCurrentLayer( int layer )
{
	ubyte blyr = (1 << layer);

	if (g_lockedLayers & blyr) return false;

	g_visibleLayers |= blyr;
	g_visibleLayersP = pointLayerMask( g_visibleLayers );

	g_currentLayer = blyr;
	g_currentLayerP = pointLayerMask( g_currentLayer );

	g_activeLayers = g_visibleLayers & ~g_lockedLayers;
	g_activeLayersP = pointLayerMask( g_activeLayers );

	s_currentEdit.updateLayerVisibility();

	outputLayerInfo();
	return true;
}

int PointEdit::getCurrentLayer() const
{
	switch (g_currentLayer)
	{
	case 0: return 7; /* hidden layer */ 
	case 1: return 0;
	case 2: return 1;
	case 4: return 2;
	case 8: return 3;
	case 16: return 4;
	case 32: return 5;
	case 64: return 6;
	default: return 0;
	}
}
bool PointEdit::isLayerLocked( int layer ) const
{
	if (g_lockedLayers & (1 << layer)) return true;
	return false;
}
bool PointEdit::isLayerVisible( int layer ) const
{
	if (g_visibleLayers & (1 << layer)) return true;
	return false;
}

//
// Tool initialisation
//
PointEdit::PointEdit()
{
	omp_set_num_threads(4);

	PointEditHistory::addFilter(s_paintSelFilter);
	PointEditHistory::addFilter(s_paintFilter);
	PointEditHistory::addFilter(s_opPaintFill);
	PointEditHistory::addFilter(s_fenceSelFilter);
	PointEditHistory::addFilter(s_planeSelFilter);
	PointEditHistory::addFilter(s_frustumSelFilter);
	PointEditHistory::addFilter(s_boxSelFilter);
	PointEditHistory::addFilter(s_orientedBoxSelFilter);
	PointEditHistory::addFilter(s_opDeselectAll);
	PointEditHistory::addFilter(s_opHideSelected);
	PointEditHistory::addFilter(s_opShowAll);
	PointEditHistory::addFilter(s_opInvertSel);
	PointEditHistory::addFilter(s_opInvertVis);
	PointEditHistory::addFilter(s_opSelectAll);
	PointEditHistory::addFilter(s_opSetLayer);
	PointEditHistory::addFilter(s_opCopyToLayer);
	PointEditHistory::addFilter(s_opMoveToLayer);
	PointEditHistory::addFilter(s_opConsolidate);
	PointEditHistory::addFilter(s_opPaintErase);
	//setUseMultiThreading();

	_units = 1.0;
}
//
bool PointEdit::moveSelToLayer( bool deselect )
{
	if (g_activeLayers & HIDDEN_LAYER_MASK)
	{
		/* move to hidden layer */ 
		hideSelPoints();
	}
	else
	{
		bool saveAutoDeselect = g_autoDeselect;
		g_autoDeselect = deselect;

		s_currentEdit.addGlobalOperation( s_opMoveToLayer );	
		setCurrentLayer( getCurrentLayer() );

		g_autoDeselect = saveAutoDeselect;
	}
	return true;
}
//
bool PointEdit::copySelToLayer( bool deselect )
{
	if (g_currentLayer == HIDDEN_LAYER_MASK) return false;

	bool saveAutoDeselect = g_autoDeselect;
	g_autoDeselect = deselect;

	s_currentEdit.addGlobalOperation( s_opCopyToLayer );
	setCurrentLayer(  getCurrentLayer() );

	g_autoDeselect = saveAutoDeselect;
		
	return true;
}
// singleton
PointEdit * PointEdit::instance()
{
	static PointEdit *_instance = 0;
	if (!_instance) _instance = new PointEdit();
	return _instance;
}

//
struct VoxelRefresh : public pointsengine::PointsVisitor
{
	bool voxel(pcloud::Voxel *vox)			
	{ 
		if (vox->flag( pcloud::PartHidden ) || vox->flag( pcloud::PartSelected ) )
		{
			PointEdit::instance()->filterVoxel( vox );	
			vox->filterPoint( vox->lodPointCount() );
		}
		return true; 
	}
};
//
void PointEdit::refreshEdit()
{
	g_editApplyMode = EditProcessFlagged ;

	/* instead set flags */ 
	FlagPartEditedVisitor fv;
	TraverseScene::withVisitor(&fv);

	ClearFilterVisitor v;
	TraverseScene::withVisitor(&v);

	ClearFlagVisitor cf;
	TraverseScene::withVisitor(&cf);

	s_currentEdit.apply();

	g_editApplyMode = EditNormal;
}
//
void PointEdit::clearEdit()
{
	s_currentEdit.clear();
	clearAll();
}
//
// paint sphere
//
void PointEdit::paintSelSphere()
{
	s_paintSelFilter.filterData.useSphere();
}
//
// paint cube
//
void PointEdit::paintSelCube()
{
	s_paintSelFilter.filterData.useBox();
}
//
// Select all
//
void PointEdit::selectAll()
{
	s_currentEdit.addGlobalOperation( s_opSelectAll );
}
//
//
//
void PointEdit::clearAll()
{
	ClearFilterVisitor v;
	TraverseScene::withVisitor(&v);
}
//
// deselect all
//
void PointEdit::deselectAll()
{
	s_currentEdit.addGlobalOperation( s_opDeselectAll );
}
//
// Show all
//
void PointEdit::showAll()
{
	s_currentEdit.addGlobalOperation( s_opShowAll );
}
//
// Invert Points
//
void PointEdit::invertSelection()
{	
	s_currentEdit.addGlobalOperation( s_opInvertSel );
}
//
// Invert Vis Points
//
void PointEdit::invertVisibility()
{	
	s_currentEdit.addGlobalOperation( s_opInvertVis );
}
//
// Hde Points
//
void PointEdit::hideSelPoints()
{
	s_currentEdit.addGlobalOperation( s_opHideSelected );
}
//
// Paint
//
void PointEdit::paintRadius( float radius )
{
	s_paintSelFilter.filterData.setRadius( radius );
}
//
//
//
float PointEdit::getPaintRadius( )
{
	return s_paintSelFilter.filterData.getRadius();
}
//
//
//
void PointEdit::setViewParams( const pt::ViewParams &params )
{
	_view = params;
	_view.updatePipeline();
}
//
// paint Select At Point
//
void PointEdit::paintSelectAtPoint( const pt::vector3 &pnt, bool limit_range )
{
	static pt::vector3 lastpoint(0,0,0);

	pt::vector3 cam;
	//if (!_camera) return;
	//_view->getLocation(cam);

	///* check for depth jump */ 
	float ddepth = lastpoint.dist(pnt);//fabs(cam.dist(pnt) - cam.dist(lastpoint));
	
	if ( !limit_range || lastpoint.is_zero() || ddepth  < s_paintSelFilter.filterData.getRadius() * 3)
	{
		s_paintSelFilter.filterData.setCenter( pnt );
		s_currentEdit.addEditOperation( s_paintSelFilter, true, true );
		s_lastTime = s_paintSelFilter.lastProcessTimeInMs();
		lastpoint = pnt;
	}
}
//
// rectangle select handler
//
void PointEdit::rectangleSelect( int l, int r, int b, int t)
{
	theRenderEngine().enableEditMode();

	Recti rect(l, t, r, b);
	rect.makeValid();

	s_frustumSelFilter.filterData.buildFromScreenRect(rect, _view, _units);
	s_currentEdit.addEditOperation(s_frustumSelFilter);
	s_lastTime = s_frustumSelFilter.lastProcessTimeInMs();
}
//
// box selection
//
void PointEdit::boxSelect( const pt::vector3 &lower, const pt::vector3 &upper )
{
	theRenderEngine().enableEditMode();

	s_boxSelFilter.filterData.box.set( lower, upper );
	s_currentEdit.addEditOperation( s_boxSelFilter );
	s_lastTime = s_frustumSelFilter.lastProcessTimeInMs();
}
//
// oriented box selection
//
void PointEdit::orientedBoxSelect( const pt::vector3 &lower, const pt::vector3 &upper, const pt::vector3 &position, const pt::vector3 &uAxis, const pt::vector3 &vAxis)
{
	theRenderEngine().enableEditMode();

	s_orientedBoxSelFilter.filterData.box.set(lower, upper);
	s_orientedBoxSelFilter.filterData.setTransform(position, uAxis, vAxis);
	s_currentEdit.addEditOperation( s_orientedBoxSelFilter );
	s_lastTime = s_frustumSelFilter.lastProcessTimeInMs();
}

//
// plane selection
//
void PointEdit::planeSelect( const pt::vector3 &origin, const pt::vector3 &normal, float thickness )
{
	theRenderEngine().enableEditMode();

	vector3d od( origin );
	vector3d nd( normal );
	s_planeSelFilter.filterData.plane.base( od );
	s_planeSelFilter.filterData.plane.normal( nd );
	s_planeSelFilter.filterData.thickness = thickness;
	s_planeSelFilter.filterData.unbounded = false;

	s_currentEdit.addEditOperation( s_planeSelFilter );
	s_lastTime = s_frustumSelFilter.lastProcessTimeInMs();
}
//
// fence selection
//
void PointEdit::fenceSelect( const pt::Fence<int> &fence )
{
	theRenderEngine().enableEditMode();

	s_fenceSelFilter.filterData.buildFromScreenFence(fence, _view, _units);
	s_currentEdit.addEditOperation(s_fenceSelFilter);
	s_lastTime = s_fenceSelFilter.lastProcessTimeInMs();
}
////
////
////
pt::datatree::Branch *PointEdit::_getEditDatatree( int index )
{
	int c=0;
	EditMap::iterator i = _edits.begin();

	while (c++ < index) ++i;

	if (i!= _edits.end())
	{
		PointEditHistory *edit = (PointEditHistory*)(i->second);
		edit->_operations->addNode( "name", i->first );
		return edit->_operations;
	}
	return 0;
}
void PointEdit::_createEditFromDatatree( pt::datatree::Branch * dtree )
{
	PointEditHistory *edit = new PointEditHistory(dtree);
	pt::String name;
	if (dtree->getNode( "name", name))
		_edits.insert(EditMap::value_type(name, (void*)edit));
}
void PointEdit::filterVoxel( pcloud::Voxel *vox )
{
	s_currentEdit.apply(vox);
}
