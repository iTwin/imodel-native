//#pragma once
//
//#include <pt/boundingbox.h>
//#include <ptengine/pointsscene.h>
//#include <ptengine/pointspager.h>
//#include <ptengine/engine.h>
//#include <ptcloud2/voxel.h>
//#include <ptgl/glviewstore.h>
//#include <pt/rect.h>
//#include <pt_edit/edit.h>
//#include <pt_edit/editnodedef.h>
//#include <pt/project.h>
//#include <pt/timestamp.h>
//#include <pt/datatree.h>
//#include <omp.h>
//
//namespace ptedit
//{
//
//enum FilterFlag
//{
//	PostFilterConsolidate = 1,
//	MultithreadCompatible = 2
//};/*
//#define	NO_TEST_POINT \
//		inline static bool testPoint(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i) { return true; } 
//
//#define NO_PAINT_POINT \
//		static void paintPoint(int t, FilterData &d, pcloud::Voxel *v, const pt::vector3d &p, uint i){}; 
//
//#define TEST_POINT \
//		static bool testPoint(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i)  \
//		{ \
//			if (!detail::g_selPntTest) return true; \
//			else return detail::g_selPntTest(t,v,p,i); \
//		}  
//
//#define FILTER_FLAGS(multithread, consolidate) \
//		bool flag(uint f) { \
//			switch (f) {	\
//				case PostFilterConsolidate: return consolidate;  \
//				case MultithreadCompatible: return multithread;  \
//			} \
//			assert(0); return false; \
//		}; 
//
//#define SELECTION_FILTER_DETAIL(multithread, consolidate) \
//		static FilterType filterType() { return SelectionFilterType; }; \
//		static FILTER_FLAGS(multithread, consolidate) \
//		NO_PAINT_POINT \
//		TEST_POINT 
//
//#define PAINT_FILTER_DETAIL(multithread, consolidate) \
//		static FilterType filterType() { return PaintFilterType; };\
//		static FILTER_FLAGS(multithread, consolidate) \
//		NO_TEST_POINT  */
//
//namespace detail
//{
//	extern SelectionMode g_mode;
//	extern bool g_autoDeselect;
//	extern bool g_multithreaded;
//
//	extern ubyte g_activeLayers;
//	extern ubyte g_activeLayersP;
//	extern ubyte g_visibleLayers;
//	extern ubyte g_visibleLayersP;
//	extern ubyte g_currentLayer;
//	extern ubyte g_currentLayerP;
//	extern float g_editDensity;
//
//	extern PointTestFunc g_selPntTest;
//
//	enum EditApplyMode
//	{
//		EditNormal = 0,
//		EditIncludeOOC = 1,
//		EditIntentRegen = 2,
//		EditIntentRefresh = 4,
//		EditIntentFastest = 8,
//		EditViewBased = 16,
//		EditIntentFlagged = 32
//	};
//	extern uint g_editApplyMode;
//	
//	struct VoxFiltering
//	{
//		static void setPoint( pcloud::Voxel *v )
//		{
//			if ( !v->numPointsEdited() || v->numPointsEdited() > v->pointCount() )
//			{
//				v->numPointsEdited( v->pointCount() * g_editDensity );
//			}
//		}		
//		
//		template < class Receiver, class Transformer >
//		static void iteratePoints( pcloud::Voxel *v, Receiver &R, Transformer &T )
//		{
//			float am = g_editApplyMode & EditIncludeOOC ? 1.0f : MIN_FILTER;
//
//			// check for override in working mode
//			if (g_editWorkingMode == EditWorkOnAll)			am = 1.0f;
//			if (g_editWorkingMode == EditWorkOnProportion)	am = g_state.density;
//
//			int fp = v->numPointsEdited();
//			{
//				pointsengine::VoxelLoader load( v, am, false, false, false );
//
//				if ( g_editApplyMode & EditIncludeOOC ) v->numPointsEdited(0);
//
//				if (g_editApplyMode & ( EditIntentRefresh | EditIntentFastest)) // CHECK, DIFFERS TO API
//				{
//					if (g_multithreaded)
//						v->_iterateTransformedPoints4Threads( R, T, false, v->numPointsEdited(), 0 );
//					else
//						v->_iterateTransformedPointsRange( R, T, false, v->numPointsEdited(), 0 );
//				}
//				else
//				{
//					if (g_multithreaded)
//						v->_iterateTransformedPoints4Threads( R, T, false, 0, v->numPointsEdited() );
//					else
//						v->_iterateTransformedPointsRange( R, T, false, 0, v->numPointsEdited() );
//				}
//			}
//			if ( g_editApplyMode & EditIncludeOOC ) 
//				v->numPointsEdited( v->fullPointCount() );
//		}
//	};
//
//	struct NodeCheck
//	{
//		inline static bool isExcluded( const pcloud::Node* n )
//		{
//			if (g_editApplyMode == EditIntentFlagged)
//			{
//				return !n->flag(pcloud::Flagged);
//			}
//			if (g_editApplyMode == EditIntentRefresh)
//			{
//				if (n->flag(pcloud::PartSelected) || n->layers(1)) 
//					return false;
//				return true;
//			}
//			return false;
//		}
//		inline static bool wholeInLayer( pcloud::Node* n, uint layer )
//		{
//			return n->layers(0) & layer;
//		}
//		inline static bool partInLayer( pcloud::Node* n, uint layer )
//		{
//			return n->layers(1) & layer;
//		}	
//	};
//
//	/* clear hidden / selection */ 
//	struct ClearFilterVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode(const pcloud::Node *n);
//	};
//	static ClearFilterVisitor s_clearFilterVisitor;
//
//	/* consolidate visitor */ 
//	struct ConsolidateVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *node );
//
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { mt_point(0 , pnt, index, f); }
//
//		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f)
//		{ 
//			if (_lyrstate != (f & ~(HIDDEN_PNT_VAL | SELECTED_PNT_VAL)) ) 
//				_consolidateLyr[t] = false;
//
//			if ( (!_selstate && (f & SELECTED_PNT_VAL))
//				|| (_selstate && !(f & SELECTED_PNT_VAL)))  
//					_consolidateSel[t] = false;
//		}
//
//		pcloud::Voxel *_currentVoxel;
//		ubyte _lyrstate;
//		bool _selstate;
//		bool _consolidateLyr[EDT_MAX_THREADS];
//		bool _consolidateSel[EDT_MAX_THREADS];
//	};
//
//	/* consolidate visitor */ 
//	struct FlagPartEditedVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *node )
//		{
//			if (node->flag(pcloud::PartSelected) || node->layers(1))
//			{
//				const_cast<pcloud::Node*>(node)->flag(pcloud::Flagged, true, true);
//				return false;
//			}
//			return node->isLeaf() ? false : true;
//		}
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { }
//		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}
//
//		pcloud::Voxel *_currentVoxel;
//	};
//	struct ClearFlagged : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *node )
//		{
//			const_cast<pcloud::Node*>(node)->flag(pcloud::Flagged, false, true);
//			return false;
//		}
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { }
//		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}
//
//		pcloud::Voxel *_currentVoxel;
//	};
//
//	/* clear flag points */ 
//	struct ClearFlagVisitor : public pcloud::Node::Visitor
//	{
//		ClearFlagVisitor(pcloud::Flag fullflag = pcloud::WholeSelected, 
//			pcloud::Flag partflag=pcloud::PartSelected, uint selval=SELECTED_PNT_VAL);
//
//		bool visitNode(const pcloud::Node *n);
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)	{ f &= ~_selval; }
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)	{ f &= ~_selval; }
//
//		pcloud::Flag _fflag;
//		pcloud::Flag _pflag;
//		uint _selval;
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Count Selected							                                */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct CountSelectedVisitor : public pcloud::Node::Visitor
//	{
//		CountSelectedVisitor() { _count[0] = _count[1] = _count[2] = _count[3] = 0; }
//		bool visitNode(const pcloud::Node *n)
//		{
//			if (n->flag(pcloud::WholeHidden)) return false;
//
//			if (n->flag(pcloud::WholeSelected))
//			{
//				if (n->flag(pcloud::PartHidden)) return true;
//				else
//				{
//					_count[0] += n->pointCount();
//					return false;
//				}
//			}
//			else if (n->flag(pcloud::PartSelected))
//			{
//				if (n->isLeaf())
//				{
//					pcloud::Voxel *v = const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n));
//					pcloud::Voxel::LocalSpaceTransform lst;
//					VoxFiltering::iteratePoints( v, *this, lst );
//				}
//				else  return true;
//			}
//			else return false;
//		}
//		inline void point( const pt::vector3d &p, uint index, ubyte &f) { mt_point(0, p, index, f);}
//		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) { 
//			if (f & SELECTED_PNT_VAL && f & g_activeLayersP) ++_count[t];
//		}
//
//		int64_t totalCount() const 
//		{ 
//			int64_t c;		
//			for (int i=0;i<EDT_MAX_THREADS;i++) c += _count[i];
//			return c;
//		}
//		int64_t _count[EDT_MAX_THREADS];
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Count Selected							                                */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct GetSelectedVisitor : public pcloud::Node::Visitor
//	{
//		GetSelectedVisitor(int maxNumPoints) 
//		{ 
//			_count=0;
//			_points = new pt::vector3[maxNumPoints]; 
//			_bufferSize = maxNumPoints;
//		}
//		~GetSelectedVisitor() { delete [] _points; }
//
//		bool visitNode(const pcloud::Node *n)
//		{
//			if (n->flag(pcloud::WholeHidden)) return false;
//
//			if (n->flag(pcloud::PartSelected) || n->flag(pcloud::PartSelected))
//			{
//				if (n->isLeaf())
//				{
//					pcloud::Voxel *v = const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n));
//					pcloud::Voxel::LocalSpaceTransform lst;
//					VoxFiltering::iteratePoints( v, *this, lst );
//				}
//				else return true;
//			}
//			else return false;
//		}
//		inline void point(const pt::vector3d &p, uint index, ubyte &f) { mt_point(0, p, index, f);}
//		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
//		{ 
//			if (f & SELECTED_PNT_VAL && f & g_activeLayersP)
//			{
//				if (_count < _bufferSize)
//					_points[++_count].set(p);
//			}		
//		}
//
//		int64_t totalCount() const { return _count; }
//		pt::vector3 *buffer() const { return _points; }
//		
//	private:
//		int64_t _count;
//		int64_t _bufferSize;
//		pt::vector3 *_points;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Move selected points to active layers                                   */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct MoveToLayerVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *n );
//		
//		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) { point(p, index, f);}
//
//		inline void point(const pt::vector3d &p, uint index, ubyte &f) { 
//			if (f & SELECTED_PNT_VAL) { f = g_currentLayerP | SELECTED_PNT_VAL; f &= ~HIDDEN_PNT_VAL; } 
//			else if (!(f & g_visibleLayersP)) f |= HIDDEN_PNT_VAL;
//		}
//		
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Copy selected points to active layers                                   */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct CopyToLayerVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *n );
//		
//		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) { point(p, index, f);}
//
//		inline void point(const pt::vector3d &p, uint index, ubyte &f) { 
//			if (f & SELECTED_PNT_VAL)  { f |= g_currentLayerP; f &= ~HIDDEN_PNT_VAL; } 
//			else if (!(f & g_visibleLayersP)) f |= HIDDEN_PNT_VAL; 
//		}
//		
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Set the layer                                                           */ 
//	/*  Hides points that are not in one of the active layers and shows points  */ 
//	/*  that are                                                                */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct UpdateLayerVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode(const pcloud::Node *n);
//
//		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) {
//			if (f & g_visibleLayersP)	{ f &= ~HIDDEN_PNT_VAL; }
//			else						{ f |= HIDDEN_PNT_VAL; }
//		}
//
//		inline void point(const pt::vector3d &p, uint index, ubyte &f) {
//			if (f & g_visibleLayersP)	{ f &= ~HIDDEN_PNT_VAL; }
//			else						{ f |= HIDDEN_PNT_VAL; }
//		}
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Set the layer                                                           */ 
//	/*  Hides points that are not in one of the active layers and shows points  */ 
//	/*  that are                                                                */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct SetHiddenLayerVisitor : public pcloud::Node::Visitor
//	{
//		SetHiddenLayerVisitor();
//		bool visitNode(const pcloud::Node *n);
//
//		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) {
//			if (f & 0x7E)		{ f |= HIDDEN_PNT_VAL; }
//			else				{ f &= ~HIDDEN_PNT_VAL; }
//		}
//		inline void point(const pt::vector3d &p, uint index, ubyte &f) {
//			if (f & 0x7E)		{ f |= HIDDEN_PNT_VAL; }
//			else				{ f &= ~HIDDEN_PNT_VAL; }
//		}
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Hide selected points in the active layers                               */ 
//	/*  Hiding means removing from those layers                                 */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct HidePointsVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *n );
//
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & SELECTED_PNT_VAL && f & g_activeLayersP)
//			{ f |= HIDDEN_PNT_VAL; f &= ~SELECTED_PNT_VAL; f &= ~g_activeLayersP; }
//		}
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			/* deselect and hide */ 
//			if (f & SELECTED_PNT_VAL && f & g_activeLayersP)
//			{ f |= HIDDEN_PNT_VAL; f &= ~SELECTED_PNT_VAL; f &= ~g_activeLayersP; }
//		}
//	private:
//		pcloud::Voxel *_currentVoxel;
//	};
//
//	/* ------------------------------------------------------------------------ */ 
//	/*  Hide selected points in the active layers                               */ 
//	/*  Hiding means removing from those layers                                 */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct DeselectPointsVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *n );
//
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & SELECTED_PNT_VAL && f & g_activeLayersP)
//			{ f &= ~SELECTED_PNT_VAL; }
//		}
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & SELECTED_PNT_VAL && f & g_activeLayersP)
//			{ f &= ~SELECTED_PNT_VAL; }
//		}
//	private:
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Select points in the active layers										*/ 
//	/* ------------------------------------------------------------------------ */ 
//	struct SelectPointsVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode( const pcloud::Node *n );
//
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & g_activeLayersP) { f |= SELECTED_PNT_VAL; }
//		}
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & g_activeLayersP) { f |= SELECTED_PNT_VAL; }
//		}
//	private:
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Hide selected points in the active layers                               */ 
//	/*  Hiding means removing from those layers                                 */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct DeselectHiddenVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode(const pcloud::Node *n);
//
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & (SELECTED_PNT_VAL | HIDDEN_PNT_VAL)) f &= ~SELECTED_PNT_VAL;
//		}
//		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & (SELECTED_PNT_VAL | HIDDEN_PNT_VAL))
//				f &= ~SELECTED_PNT_VAL;
//		}
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Invert points selected state                                            */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct InvertSelectionVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode(const pcloud::Node *n);
//
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & g_activeLayersP)
//			{
//				if (f & SELECTED_PNT_VAL) f &= ~SELECTED_PNT_VAL;
//				else f |= SELECTED_PNT_VAL;
//			}
//		}
//		void point(const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & g_activeLayersP)
//			{
//				if (f & SELECTED_PNT_VAL) f &= ~SELECTED_PNT_VAL;
//				else f |= SELECTED_PNT_VAL;
//			}
//		}
//	private:
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Invert points visibility state within the layer                         */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct InvertVisibilityVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode(const pcloud::Node *n);
//
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & g_activeLayersP)	{ f &= ~g_activeLayersP; f |= HIDDEN_PNT_VAL; }
//			else { f |= g_activeLayersP; f &= ~HIDDEN_PNT_VAL; }
//		}
//		void point(const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			if (f & g_activeLayersP)	{ f &= ~g_activeLayersP; f |= HIDDEN_PNT_VAL; }
//			else { f |= g_activeLayersP; f &= ~HIDDEN_PNT_VAL; }
//		}
//	private:
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Invert points visibility state within the layer                         */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct UnhideVisitor : public pcloud::Node::Visitor
//	{
//		bool visitNode(const pcloud::Node *n);
//
//		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			f |= g_activeLayersP; f &= ~HIDDEN_PNT_VAL;
//		}
//		void point(const pt::vector3d &pnt, uint index, ubyte &f)
//		{
//			f |= g_activeLayersP; f &= ~HIDDEN_PNT_VAL;
//		}
//	private:
//		pcloud::Voxel *_currentVoxel;
//	};
//	/* ------------------------------------------------------------------------ */ 
//	/*  Traverse the whole scene                                                */ 
//	/* ------------------------------------------------------------------------ */ 
//	struct TraverseScene
//	{
//		static void withVisitor(pcloud::Node::Visitor *v)
//		{
//			int numScenes = pointsengine::thePointsScene().size();
//			for (int sc=0; sc<numScenes; sc++)
//			{
//				pcloud::Scene* scene = pointsengine::thePointsScene()[sc];
//
//				int numClouds = scene->size();
//				for (int cl=0; cl<numClouds; cl++)
//					const_cast<pcloud::Node*>(scene->cloud(cl)->root())->traverseTopDown(v);
//			}
//		}
//		static void withFlag(uint flag, bool value)
//		{
//			int numScenes = pointsengine::thePointsScene().size();
//			for (int sc=0; sc<numScenes; sc++)
//			{
//				pcloud::Scene* scene = pointsengine::thePointsScene()[sc];
//
//				int numClouds = scene->size();
//				for (int cl=0; cl<numClouds; cl++)
//					const_cast<pcloud::Node*>(scene->cloud(cl)->root())->flag(flag, value, true);
//			}
//		}
//	};
//}
////
//// Filter base class template
////
//template <typename PointFilter>
//struct SelectionFilter 
//{
//public:
//	typedef PointFilter PointFilterType;
//
//	/* updated impl */ 
//	bool apply()
//	{
//		return processFilter();
//	}
//	const char *name() const { return filterData.name(); }
//
//	/* old impl */ 
//	void loadStateBranch(const pt::datatree::Branch *b)
//	{
//		filterData.read(b);
//	}
//
//	void loadStateBranchAndProcess(const pt::datatree::Branch *b)
//	{
//		loadStateBranch(b);
//		processFilter();
//	}
//
//	bool flag(uint f) { return PointFilter::flag(f); }
//
//	// for points export 
//	void processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
//	{	
//		_basePoint = pt::vector3(Project3D::project().registration().matrix()(3,0), 
//				Project3D::project().registration().matrix()(3,1), 
//				Project3D::project().registration().matrix()(3,2));
//
//			pcloud::Voxel::CoordinateSpaceTransform cst(
//				const_cast<pcloud::PointCloud*>(voxel->pointCloud()));
//
//		processNode(voxel, cst);
//		return;
//	}
//	// process this filter through the point data 
//	void processFilter()
//	{
//		_didSelect = 0;
//
//		pointsengine::theRenderEngine().enableEditMode();
//
//		pt::TimeStamp t0, t1;
//		t0.tick();
//
//		_basePoint = pt::vector3(Project3D::project().registration().matrix()(3,0), 
//				Project3D::project().registration().matrix()(3,1), 
//				Project3D::project().registration().matrix()(3,2));
//
//		int numScenes = pointsengine::thePointsScene().size();
//		for (int sc=0; sc<numScenes; sc++)
//		{
//			pcloud::Scene* scene = pointsengine::thePointsScene()[sc];
//			const pt::Bounds3D &bounds = scene->projectBounds();
//			int numClouds = scene->size();
//			int cl;
//			
//			/* test Bounds */ 
//			SelectionResult res = PointFilter::intersect(filterData, bounds.bounds());
//
//			if (res == FullyInside && !g_selPntTest)
//			{
//				for (cl=0;cl<numClouds;cl++)
//				{
//					pcloud::PointCloud *pc = scene->cloud(cl);
//					pcloud::Node* root = const_cast<Node*>(pc->root());
//
//					if (root->layers(1) & g_activeLayers || g_mode == UnhidePoint)
//					{
//						pcloud::Voxel::CoordinateSpaceTransform cst(pc);
//						processNode( root, cst);
//					}
//					else if (root->layers(0) & g_activeLayers)
//					{
//						root->flag(pcloud::WholeSelected, g_mode == SelectPoint ? true : false, true);
//						root->flag(pcloud::PartSelected, false, true);
//						++_didSelect;
//					}
//				}
//			}
//			else if (res != FullyOutside )
//			{
//				if (res == PartiallyInside || (g_selPntTest && res == FullyInside))
//				{	
//					for (cl=0;cl<numClouds;cl++)
//					{
//						pcloud::PointCloud *pc = scene->cloud(cl);
//						pcloud::Voxel::CoordinateSpaceTransform cst(pc);
//
//						processNode(const_cast<Node*>(pc->root()), cst);
//					}
//				}
//			}
//		}
//
//		t1.tick();
//		_lastProcessTime = t1.delta_ms(t0, t1);
//	}
//
//	// return the last processing time
//	int lastProcessTimeInMs() const { return _lastProcessTime; }
//
//	
//	//
//	inline void point(const pt::vector3d &pnt, uint index, ubyte &f)	
//	{
//		static pcloud::Voxel *lvoxel=(Voxel*)1;
//
//		if (PointFilter::inside(0, filterData, pnt))
//		{
//			switch(PointFilter::filterType())
//			{
//			case SelectionFilterType:
//				if (PointFilter::testPoint(0, _currentVoxel, pnt, index))
//					selectPointInVoxel(f); 
//				break;
//			case PaintFilterType:
//			
//				if(f & g_activeLayersP)
//					PointFilter::paintPoint(0, filterData, _currentVoxel, pnt, index); 
//				break;
//			}
//		}
//	}
//
//	//
//	inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)	
//	{
//		
//		if (PointFilter::inside(thread, filterData, pnt))
//		{
//			switch(PointFilter::filterType())
//			{
//			case SelectionFilterType:
//				if (PointFilter::testPoint(thread, _currentVoxel, pnt, index))
//					selectPointInVoxel(f); 
//				break;
//			case PaintFilterType:
//				if(f & g_activeLayersP)
//					PointFilter::paintPoint(thread, filterData, _currentVoxel, pnt, index);
//				break;
//			}
//		}
//	}
//
//	typename PointFilter::FilterData	filterData;
//
//	int didSelect() const { return _didSelect; }
//
//protected:
//	int				_lastProcessTime;
//	pt::vector3		_basePoint;
//	pcloud::Voxel	*_currentVoxel;
//	int				_didSelect;
//
//	inline void selectPointInVoxel(ubyte &f)
//	{	
//		switch (detail::g_mode)
//		{
//		case SelectPoint:
//			if (f & g_activeLayersP)
//			{
//				f |= SELECTED_PNT_VAL;
//				++_didSelect;
//			}
//			break;
//
//		case DeselectPoint:
//			if (f & g_activeLayersP)
//			{
//				f &= ~SELECTED_PNT_VAL;
//				++_didSelect;
//			}
//			break;
//
//		case UnhidePoint:
//			if (!(f & g_activeLayersP))	
//			{
//				f &= ~HIDDEN_PNT_VAL;
//				f |= g_activeLayersP;
//				++_didSelect;
//			}
//			break;
//		}
//	}
//
//private:
//	std::vector<pcloud::Voxel *> _voxellist;
//
//	//
//	void processNode(const pcloud::Node* node, pcloud::Voxel::CoordinateSpaceTransform &cst)
//	{
//		if (NodeCheck::isExcluded(node)) return;
//
//		BoundingBox box = node->extents();
//		box.translateBy(-_basePoint);
//
//		SelectionResult res = PointFilter::intersect(filterData, box);
//
//		if (!res) return;
//		pcloud::Node *n = const_cast<pcloud::Node*>(node);
//
//		if (PointFilter::filterType() ==  SelectionFilterType)
//		{
//			/* Unhide point mode */ 
//			if (g_mode == UnhidePoint)
//			{
//				if ( n->layers(0) & g_activeLayers )
//					return;
//				
//				else if (res == PartiallyInside || n->layers(1) & g_activeLayers)
//				{
//					if (n->isLeaf())
//					{
//						_currentVoxel = static_cast<pcloud::Voxel*>(const_cast<pcloud::Node*>(node));
//
//						ubyte initial_val = 0;
//						if ( _currentVoxel->flag(pcloud::WholeSelected) ) initial_val |= SELECTED_PNT_VAL;
//						if ( _currentVoxel->flag(pcloud::WholeHidden) ) initial_val |= HIDDEN_PNT_VAL;
//						
//						/* if currently fully selected, no filter channel will exist. We need to fill a channel with sel value */ 
//						_currentVoxel->buildFilterChannel( initial_val );
//
//						boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
//
//						/* load a minimal number of points if nothing is loaded to ensure we set a filter point */ 
//						VoxFiltering::iteratePoints( _currentVoxel, *this, cst );
//						VoxFiltering::setPoint( _currentVoxel );
//					}
//					else
//					{
//						const pcloud::Node * n=0;
//
//						for (int oct=0; oct<8;oct++)
//						{
//							n = node->child(oct);
//							if (n)	processNode(n, cst);
//						}
//					}
//				}
//				else
//				{
//					UnhideVisitor v;
//					n->traverseTopDown(&v);
//				}
//				return;
//			}
//		}
//		
//		if ( !(n->layers(0) & g_activeLayers) && !(n->layers(1) & g_activeLayers))
//			return;
//
//		/* full inclusion */ 
//		if (res == FullyInside && n->layers(0) & g_activeLayers && 
//			PointFilter::filterType() ==  SelectionFilterType
//			&& !g_selPntTest)
//		{
//			_didSelect += n->pointCount();
//
//			switch (g_mode)
//			{
//			case SelectPoint:
//				n->flag(pcloud::WholeSelected, true, true);
//				n->flag(pcloud::PartSelected, false, true);
//				break;
//			case DeselectPoint:
//				{
//					DeselectPointsVisitor v;
//					n->traverseTopDown(&v);
//				}
//				break;
//			}
//		}
//		/* part inclusion */ 
//		else
//		{
//			if (node->isLeaf())
//			{
//				_currentVoxel = static_cast<pcloud::Voxel*>(const_cast<pcloud::Node*>(node));;	
//				
//				ubyte initial_val = _currentVoxel->layers(0) << 1;
//				if ( _currentVoxel->flag(pcloud::WholeSelected) ) initial_val |= SELECTED_PNT_VAL;
//				//if ( _currentVoxel->flag(pcloud::WholeHidden) ) initial_val |= HIDDEN_PNT_VAL;
//				/* if currently fully selected, no filter channel will exist. We need to fill a channel with sel value */ 
//				_currentVoxel->buildFilterChannel( initial_val );
//
//				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
//				/* load a minimal number of points if nothing is loaded to ensure we set a filter point */ 
//
//				VoxFiltering::iteratePoints( _currentVoxel, *this, cst );
//				VoxFiltering::setPoint( _currentVoxel );
//			}
//			else
//			{
//				/* child  */ 
//				const pcloud::Node * n=0;
//
//				for (int oct=0; oct<8;oct++)
//				{
//					n = node->child(oct);
//					if (n)
//					{
//						/* no need to iterate if already entirely selected */ 
//						if (n->isLeaf() && 
//							(
//							(node->flag(pcloud::WholeSelected) && g_mode == SelectPoint)
//							|| (g_mode == DeselectPoint && !(node->flag(pcloud::WholeSelected) || node->flag(pcloud::PartSelected)))
//							))
//						{
//							return;
//						}
//						processNode(n, cst);
//					}
//				}
//			}
//			/* set flags */ 
//			const_cast<pcloud::Node*>(node)->flag(pcloud::WholeSelected, false);
//			const_cast<pcloud::Node*>(node)->flag(pcloud::PartSelected, true);
//		}
//	}
//};
//}; /* namespace */ 