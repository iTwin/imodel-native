#pragma once
#include <ptedit/editState.h>
#include <pt/boundingbox.h>

#include <ptengine/pointsscene.h>
#include <ptengine/engine.h>
#include <ptengine/clipManager.h>
#include <ptcloud2/voxel.h>
#include <ptedit/editApply.h>
#include <ptedit/isolationFilter.h>
#include <pt/datatree.h>
#include <pt/rect.h>


namespace ptedit
{
	/* ------------------------------------------------------------------------ */ 
	/*  Traverse the whole scene                                                */ 
	/* ------------------------------------------------------------------------ */ 
	struct TraverseScene
	{
		static void withVisitor(pcloud::Node::Visitor *v, pcloud::Scene *scene, bool use_scope=true)
		{
			if (scene)
			{
				int numClouds = scene->size();
				for (int cl=0; cl<numClouds; cl++)
				{
					if ( use_scope && !g_state.inScope( scene->cloud(cl) )) continue;

					const_cast<pcloud::Node*>(scene->cloud(cl)->root())->traverseTopDown(v);
				}
			}
		}

		static void withVisitor(pcloud::Node::Visitor *v, bool use_scope=true)
		{
			int numScenes = pointsengine::thePointsScene().size();
			for (int sc=0; sc<numScenes; sc++)
			{
				pcloud::Scene* scene = pointsengine::thePointsScene()[sc];

				if ( use_scope && !g_state.inScope(scene) ) continue;
				
				withVisitor( v, scene, use_scope );
			}
		}

		static void withFlag(uint flag, bool value)
		{
			int numScenes = pointsengine::thePointsScene().size();
			for (int sc=0; sc<numScenes; sc++)
			{
				pcloud::Scene* scene = pointsengine::thePointsScene()[sc];

				if (!g_state.inScope( scene )) continue;

				int numClouds = scene->size();
				for (int cl=0; cl<numClouds; cl++)
				{
					if (!g_state.inScope( scene->cloud(cl) )) continue;
	
					const_cast<pcloud::Node*>(scene->cloud(cl)->root())->flag(flag, value, true);
				}
			}
		}
		static void consolidateFlags()
		{
			int numScenes = pointsengine::thePointsScene().size();
			for (int sc=0; sc<numScenes; sc++)
			{
				pcloud::Scene* scene = pointsengine::thePointsScene()[sc];

				int numClouds = scene->size();
				for (int cl=0; cl<numClouds; cl++)
					const_cast<pcloud::Node*>(scene->cloud(cl)->root())->recursiveFlagsConsolidate();
			}
		}
		static void consolidateLayers()
		{
			int numScenes = pointsengine::thePointsScene().size();
			for (int sc=0; sc<numScenes; sc++)
			{
				pcloud::Scene* scene = pointsengine::thePointsScene()[sc];

				int numClouds = scene->size();
				for (int cl=0; cl<numClouds; cl++)
					const_cast<pcloud::Node*>(scene->cloud(cl)->root())->recursiveLayerConsolidate();
			}
		}
	};

	class IsolationFilteredVistor : public pcloud::Node::Visitor
	{
	public:
		IsolationFilteredVistor() :
		  _isolationFilterIntersect(pointsengine::ClipManager::instance().getIntersectFunction()),
		  _isolationFilterInside(pointsengine::ClipManager::instance().getInsideFunction())	
		{			
		}

		SelectionResult intersectsIsolationFilter(pt::BoundingBoxD& box)
		{
			if (!pointsengine::ClipManager::instance().clippingEnabled())
				return FullyInside;

			return _isolationFilterIntersect(box);
		}

		bool insideIsolationFilter(int thread, const pt::vector3d& pnt)
		{
			if (!pointsengine::ClipManager::instance().clippingEnabled())
				return true;

			// voxel points are already in project space so no conversion is needed
			pt::vector3d points[EDT_MAX_THREADS];
			points[thread] = pnt;
			return _isolationFilterInside(thread, points[thread]);
		}

	protected:
		ptedit::IsolationFilter::IntersectCallback	_isolationFilterIntersect;
		ptedit::IsolationFilter::InsideCallback		_isolationFilterInside;

	};

	/* destroy filter channels */ 
	struct DestroyFilterVisitor : public pcloud::Node::Visitor
	{
		bool visitNode(const pcloud::Node *n);
	};
	static DestroyFilterVisitor s_destroyFilterVisitor;

	/* clear hidden / selection */ 
	struct ClearFilterVisitor : public pcloud::Node::Visitor
	{
		bool visitNode(const pcloud::Node *n);
	};
	static ClearFilterVisitor s_clearFilterVisitor;

	/* consolidate visitor */ 
	struct ConsolidateVisitor : public pcloud::Node::Visitor
	{
		ConsolidateVisitor( bool allLayers, bool flagState );

		bool visitNode( const pcloud::Node *node );

		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { mt_point(0 , pnt, index, f); }

		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f)
		{ 
			_pntLyrState[t] |= f;

			if (_lyrstate != (f & ~SELECTED_PNT_BIT) ) 
				_consolidateLyr[t] = false;

			if ( (!_selstate && (f & SELECTED_PNT_BIT))
				|| (_selstate && !(f & SELECTED_PNT_BIT)))  
					_consolidateSel[t] = false;
			
			_hassel |= (f & SELECTED_PNT_BIT) != 0;
		}

		pcloud::Voxel *_currentVoxel;
		ubyte _lyrstate;
		ubyte _pntLyrState[EDT_MAX_THREADS];
		bool _selstate;
		bool _hassel;
		bool _doSelState;
		bool _doAllLayers;
		bool _consolidateLyr[EDT_MAX_THREADS];
		bool _consolidateSel[EDT_MAX_THREADS];
	};
	/* update the hidden state of nodes based on layer */ 
	struct UpdateLayerVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *node );
	};

	/* Sets the layer to the hidden layer */ 
	struct SetHiddenLayerVisitor : public pcloud::Node::Visitor
	{
		SetHiddenLayerVisitor();
		bool visitNode( const pcloud::Node *node );		
	};


	struct UnflagVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *node )
		{
			const_cast<pcloud::Node*>(node)->flag(pcloud::Flagged, false, true);
			return false;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { }
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}

		pcloud::Voxel *_currentVoxel;
	};

	/* consolidate visitor */ 
	struct FlagPartEditedVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *node )
		{
			if (node->isLeaf())
			{
				// check despite flags this actually has an edit channel
				if (!static_cast<const pcloud::Voxel*>(node)->channel(pcloud::PCloud_Filter))
				{
					return false;
				}
			}		
			if (node->flag(pcloud::PartSelected) || node->layers(1) || node->flag(pcloud::Painted))
			{
				const_cast<pcloud::Node*>(node)->flag(pcloud::Flagged, true, false);
				return true;
			}
			else const_cast<pcloud::Node*>(node)->flag(pcloud::Flagged, false, false);
			
			return node->isLeaf() ? false : true;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { }
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}

		pcloud::Voxel *_currentVoxel;
	};
	//
	struct FlagPartEditedAndVisibleVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *node )
		{
			if (
				!(node->layers(0) & g_visibleLayers || 
				node->layers(1) & g_visibleLayers )) return false;

			if (node->flag(pcloud::PartSelected) || node->layers(1) || node->flag(pcloud::Painted))
			{
				const_cast<pcloud::Node*>(node)->flag(pcloud::Flagged, true, true);
				return false;
			}
			return node->isLeaf() ? false : true;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { }
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}

		pcloud::Voxel *_currentVoxel;
	};
	//
	struct ClearFlagged : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *node )
		{
			const_cast<pcloud::Node*>(node)->flag(pcloud::Flagged, false, true);
			return false;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { }
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}

		pcloud::Voxel *_currentVoxel;
	};

	/* clear flag on voxels that have a filter channel and are fully processed already */ 
	struct ClearFlaggedIfProcessed : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *node )
		{
			if (node->isLeaf())
			{
				pcloud::Voxel *vox = static_cast<pcloud::Voxel*>(const_cast<pcloud::Node*>(node));
				
				if (vox->fullPointCount() == vox->numPointsEdited() && 
					(vox->channel( pcloud::PCloud_Filter ) || vox->flag( pcloud::Painted ) ))
					vox->flag( pcloud::Flagged, false );

				//hack: check for optimisation later
				if (vox->flag( pcloud::Painted )) vox->flag( pcloud::Flagged, true );
			}
			return true;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) { }
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f) {}

		pcloud::Voxel *_currentVoxel;
	};
	/* clear flag points */ 
	struct ClearFlagVisitor : public pcloud::Node::Visitor
	{
		ClearFlagVisitor(pcloud::Flag fullflag = pcloud::WholeSelected, 
			pcloud::Flag partflag=pcloud::PartSelected, uint selval=SELECTED_PNT_BIT);

		bool visitNode(const pcloud::Node *n);
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)	{ f &= ~_selval; }
		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)	{ f &= ~_selval; }

		pcloud::Flag _fflag;
		pcloud::Flag _pflag;
		uint _selval;
		pcloud::Voxel *_currentVoxel;
	};

	/* ------------------------------------------------------------------------ */ 
	/*  Count Selected							                                */ 
	/* ------------------------------------------------------------------------ */ 
	struct CountVisibleVisitor : public pcloud::Node::Visitor
	{
		CountVisibleVisitor() { _count[0] = _count[1] = _count[2] = _count[3] = 0; }
		bool visitNode(const pcloud::Node *n)
		{
			// make sure point count is up to date
			if (!n->parent())
				const_cast<pcloud::Node*>(n)->calcLodPointCount();

			if (n->flag(pcloud::WholeHidden)) return false;

			if (!n->flag(pcloud::PartHidden))
			{
				_count[0] += n->lodPointCount();	// changed from pointCount
				return false;						// no need to continue, lodpointcount is recursive
			}
			else 
			{
				if (n->isLeaf())
				{
					pcloud::Voxel *v = const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n));

					if (v->channel( pcloud::PCloud_Filter ))
					{
						pcloud::Voxel::LocalSpaceTransform lst;
						VoxFiltering::iteratePoints( v, *this, lst );
					}
					else _count[0] += n->lodPointCount();
				}
				else  return true;
			}
			return true;
		}
		inline void point( const pt::vector3d &p, uint index, ubyte &f) { mt_point(0, p, index, f);}
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & g_activeLayers) ++_count[t];
		}

		__int64 totalCount() const 
		{ 
			__int64 c=0;		
			for (int i=0;i<EDT_MAX_THREADS;i++) c += _count[i];
			return c;
		}
		__int64 _count[EDT_MAX_THREADS];
	};

	
	/* ------------------------------------------------------------------------ */ 
	/*  Count Points in Layer					                                */ 
	/* ------------------------------------------------------------------------ */ 
	struct CountPointsInLayerVisitor : public pcloud::Node::Visitor
	{
		CountPointsInLayerVisitor( ubyte layers ) 
		{ 
			_count[0] = _count[1] = _count[2] = _count[3] = 0; 
			_layers = layers;
		}

		bool visitNode(const pcloud::Node *n)
		{			
			if ( ! 
				((n->layers(0) | n->layers(1)) & _layers)) 
			{
				return false;
			}

			if (!(n->layers(1) & _layers))
			{
				_count[0] += n->fullPointCount();	// changed from pointCount
				return false;						// no need to continue, fullpointcount is recursive
			}
			else 
			{
				if (n->isLeaf())
				{
					pcloud::Voxel *v = const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n));
					const pcloud::DataChannel *layers = v->channel( pcloud::PCloud_Filter );

					if (layers)
					{
						uint voxel_count = 0;

						// explicitly code the iteration here, because the filter channel maybe more complete than the lod
						// in the case that a channel was loaded
						if (v->numPointsEdited()==0 && layers->size())
						{
							for (unsigned int i=0; i<layers->size();i++)
							{
								if (layers->data()[i] & _layers) 
								{
									++voxel_count;
								}
							}
						}
						else
						{
							CountPointsInLayerVisitor cp(_layers); // need this to have a separate count to scale up

							pcloud::Voxel::LocalSpaceTransform lst;
							VoxFiltering::iteratePoints( v, cp, lst );
						
							// scale up
							voxel_count = cp.totalCount();

							if (v->numPointsEdited())
								voxel_count *= (float)v->fullPointCount()/v->numPointsEdited();
						}
						_count[0] += voxel_count;
					}
					else _count[0] += n->fullPointCount();	// should not happen, but hey
				}
				else  return true;
			}
			return true;
		}
		inline void point( const pt::vector3d &p, uint index, ubyte &f) { mt_point(0, p, index, f);}
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & _layers) ++_count[t];
		}

		__int64 totalCount() const 
		{ 
			__int64 c=0;		
			for (int i=0;i<EDT_MAX_THREADS;i++) c += _count[i];
			return c;
		}

		__int64 _count[EDT_MAX_THREADS];
		ubyte	_layers;
	};

	/* ------------------------------------------------------------------------ */ 
	/*  Count Selected							                                */ 
	/* ------------------------------------------------------------------------ */ 
	struct CountSelectedVisitor : public pcloud::Node::Visitor
	{
		CountSelectedVisitor() { _count[0] = _count[1] = _count[2] = _count[3] = 0; }
		bool visitNode(const pcloud::Node *n)
		{
			if (n->flag(pcloud::WholeHidden)) return false;

			if (n->flag(pcloud::WholeSelected))
			{
				if (n->flag(pcloud::PartHidden)) return true;
				else
				{
					_count[0] += n->lodPointCount();	//changed from pointCount
					return false;
				}
			}
			else if (n->flag(pcloud::PartSelected))
			{
				if (n->isLeaf())
				{
					pcloud::Voxel *v = const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n));
					pcloud::Voxel::LocalSpaceTransform lst;
					VoxFiltering::iteratePoints( v, *this, lst );
				}
				else  return true;
			}
			else return false;
		}
		inline void point( const pt::vector3d &p, uint index, ubyte &f) { mt_point(0, p, index, f);}
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) { 
			if (f & SELECTED_PNT_BIT && f & g_activeLayers) ++_count[t];
		}

		__int64 totalCount() const 
		{ 
			__int64 c=0;	
			for (int i=0;i<EDT_MAX_THREADS;i++) c += _count[i];
			return c;
		}
		__int64 _count[EDT_MAX_THREADS];
	};

	/* ------------------------------------------------------------------------ */ 
	/*  Count Selected							                                */ 
	/* ------------------------------------------------------------------------ */ 
	struct ComputeLayerBoundsVisitor : public pcloud::Node::Visitor
	{
		ComputeLayerBoundsVisitor( ubyte layerMask, bool approx );

		bool visitNode(const pcloud::Node *n);

		inline void point( const pt::vector3d &p, uint index, ubyte &f) {  mt_point(0, p, index, f); }	

		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & _layerMask)
				_bounds[t].expand( p );
		}

		pt::BoundingBoxD boundingBox() const 
		{ 
			pt::BoundingBoxD bb;		
			for (int i=0;i<EDT_MAX_THREADS;i++) 
				if (!_bounds[i].isEmpty())
					bb.expandBy( _bounds[i] );
			return bb;
		}
		bool				_approx;
		ubyte				_layerMask;		
		pt::vector3d		_basepoint;
		pt::BoundingBoxD	_bounds[EDT_MAX_THREADS];
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Check for occupancy						                                */ 
	/* ------------------------------------------------------------------------ */ 
	struct DoesLayerHavePointsVisitor : public pcloud::Node::Visitor
	{
		DoesLayerHavePointsVisitor( ubyte layerMask, bool approx ) 
		{ 
			_hasPnt = false; _approx = approx; _layerMask = layerMask; 
		}

		bool visitNode(const pcloud::Node *n);

		inline void point( const pt::vector3d &p, uint index, ubyte &f) {  mt_point(0, p, index, f); }	

		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & _layerMask)
				_hasPnt = true;
		}
		bool hasPoints() const	{ return _hasPnt; }
		void approx( bool a )	{ _approx = a; }

		bool				_approx;
		ubyte				_layerMask;
		bool				_hasPnt;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Count Selected							                                */ 
	/* ------------------------------------------------------------------------ */ 
	struct GetSelectedVisitor : public pcloud::Node::Visitor
	{
		GetSelectedVisitor(int maxNumPoints) 
		{ 
			_count=0;
			_points = new pt::vector3[maxNumPoints]; 
			_bufferSize = maxNumPoints;
		}
		~GetSelectedVisitor() { delete [] _points; }

		bool visitNode(const pcloud::Node *n)
		{
			if (n->flag(pcloud::WholeHidden)) return false;

			if (n->flag(pcloud::PartSelected) || n->flag(pcloud::PartSelected))
			{
				if (n->isLeaf())
				{
					pcloud::Voxel *v = const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n));
					pcloud::Voxel::LocalSpaceTransform lst;
					VoxFiltering::iteratePoints( v, *this, lst );
				}
				else return true;
			}
			else return false;
		}
		inline void point(const pt::vector3d &p, uint index, ubyte &f) { mt_point(0, p, index, f);}
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & SELECTED_PNT_BIT && f & g_activeLayers)
			{
				if (_count < _bufferSize)
					_points[++_count].set(p);
			}		
		}

		__int64 totalCount() const { return _count; }
		pt::vector3 *buffer() const { return _points; }
		
	private:
		__int64 _count;
		__int64 _bufferSize;
		pt::vector3 *_points;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Move selected points to active layers                                   */ 
	/* ------------------------------------------------------------------------ */ 
	struct MoveToLayerVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *n );
		
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) { point(p, index, f);}

		inline void point(const pt::vector3d &p, uint index, ubyte &f) { 
			if (f & SELECTED_PNT_BIT) { f = g_currentLayer | SELECTED_PNT_BIT; } 
		}
		
		pcloud::Voxel *_currentVoxel;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Copy selected points to active layers                                   */ 
	/* ------------------------------------------------------------------------ */ 
	struct CopyToLayerVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *n );
		
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) { point(p, index, f);}

		inline void point(const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & SELECTED_PNT_BIT)  
			{ 
				f |= g_currentLayer; 
} 
		}
		
		pcloud::Voxel *_currentVoxel;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Hide selected points in the active layers                               */ 
	/*  Hiding means removing from those layers                                 */ 
	/* ------------------------------------------------------------------------ */ 
	struct HidePointsVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *n );

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (f & SELECTED_PNT_BIT && f & g_activeLayers)
			{ 
				f &= ~SELECTED_PNT_BIT; 
				f &= ~g_activeLayers; 
			}
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			/* deselect and hide */ 
			if (f & SELECTED_PNT_BIT && f & g_activeLayers)
			{ 
				f &= ~SELECTED_PNT_BIT; 
				f &= ~g_activeLayers; 
			}
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};

	/* ------------------------------------------------------------------------ */ 
	/*  Hide selected points in the active layers                               */ 
	/*  Hiding means removing from those layers                                 */ 
	/* ------------------------------------------------------------------------ */ 
	struct DeselectPointsVisitor : public IsolationFilteredVistor
	{
		bool visitNode( const pcloud::Node *n );

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if ((f & SELECTED_PNT_BIT) 
				&& (f & g_activeLayers)
				&& insideIsolationFilter(thread, pnt))
			{ 
				f &= ~SELECTED_PNT_BIT; 
			}
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (f & SELECTED_PNT_BIT 
			    && (f & g_activeLayers)
				&& insideIsolationFilter(0, pnt))
			{ 
				f &= ~SELECTED_PNT_BIT; 
			}
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};

	/* ------------------------------------------------------------------------ */ 
	/*  Hide selected points in the active layers                               */ 
	/*  Hiding means removing from those layers                                 */ 
	/* ------------------------------------------------------------------------ */ 
	struct DeselectAnyPointsVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *n );

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			f &= ~SELECTED_PNT_BIT;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			f &= ~SELECTED_PNT_BIT;
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};
	/* ------------------------------------------------------------------------ */ 
	/* ------------------------------------------------------------------------ */ 
	struct SelectPointsInLayerVisitor : public IsolationFilteredVistor
	{
		SelectPointsInLayerVisitor(int layerIndex )			
		{
			_targetLayer = 1 << layerIndex;
		}
		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (f & _targetLayer && insideIsolationFilter(thread, pnt)) 
			{
				f |= SELECTED_PNT_BIT; 										
			}
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			mt_point(0, pnt, index, f);
		}
		bool visitNode( const pcloud::Node *n );
	private:
		pcloud::Voxel *_currentVoxel;
		ubyte			_targetLayer;	
	};

	/* ------------------------------------------------------------------------ */ 
	/* ------------------------------------------------------------------------ */ 
	struct DeselectPointsInLayerVisitor : public IsolationFilteredVistor
	{
		DeselectPointsInLayerVisitor(int layerIndex)			
		{
			_targetLayer = 1 << layerIndex;
		}
		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if ((f & _targetLayer) && insideIsolationFilter(thread, pnt)) 
			{
				f &= ~SELECTED_PNT_BIT; 										
			}
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			mt_point(0, pnt, index, f);
		}
		bool visitNode( const pcloud::Node *n );
	private:
		pcloud::Voxel *_currentVoxel;
		ubyte			_targetLayer;	
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Hide selected points in the active layers                               */ 
	/*  Hiding means removing from those layers                                 */ 
	/* ------------------------------------------------------------------------ */ 
	struct SelectPointsVisitor : public IsolationFilteredVistor
	{
		bool visitNode( const pcloud::Node *n );

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (( f & g_activeLayers) && insideIsolationFilter(thread, pnt))
			{ 
				f |= SELECTED_PNT_BIT; 
			}
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (( f & g_activeLayers) && insideIsolationFilter(0, pnt)) 
			{ 
				f |= SELECTED_PNT_BIT; 
			}
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};

	/* ------------------------------------------------------------------------ */ 
	/*  Deselects points that are hidden									    */ 
	/*  to avoid selected state when shown									    */ 
	/* ------------------------------------------------------------------------ */ 
	struct DeselectHiddenVisitor : public pcloud::Node::Visitor
	{
		bool visitNode(const pcloud::Node *n);

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (!(f & g_activeLayers)) f &= ~SELECTED_PNT_BIT;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (!(f & g_activeLayers)) f &= ~SELECTED_PNT_BIT;
		}
		pcloud::Voxel *_currentVoxel;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Invert points selected state                                            */ 
	/* ------------------------------------------------------------------------ */ 
	struct InvertSelectionVisitor : public IsolationFilteredVistor
	{
		bool visitNode(const pcloud::Node *n);

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if ((f & g_activeLayers) 
				&& insideIsolationFilter(thread, pnt))
			{
				if (f & SELECTED_PNT_BIT) f &= ~SELECTED_PNT_BIT;
				else f |= SELECTED_PNT_BIT;
			}
		}
		void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if ((f & g_activeLayers) 
				&& insideIsolationFilter(0, pnt))
			{
				if (f & SELECTED_PNT_BIT) f &= ~SELECTED_PNT_BIT;
				else f |= SELECTED_PNT_BIT;
			}
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Isolate points				                                            */ 
	/* ------------------------------------------------------------------------ */ 
	struct IsolateSelectedVisitor : public pcloud::Node::Visitor
	{
		bool visitNode(const pcloud::Node *n);

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (f & g_activeLayers)
			{
				if (f & SELECTED_PNT_BIT) f &= ~SELECTED_PNT_BIT;
				else f &= ~g_activeLayers;
			}
		}
		void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (f & g_activeLayers)
			{
				if (f & SELECTED_PNT_BIT) f &= ~SELECTED_PNT_BIT;
				else f &= ~g_activeLayers;
			}		
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Invert points visibility state within the layer                         */ 
	/* ------------------------------------------------------------------------ */ 
	struct InvertVisibilityVisitor : public pcloud::Node::Visitor
	{
		bool visitNode(const pcloud::Node *n);

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (f & g_activeLayers)	f &= ~g_activeLayers; 
			else					f |= g_activeLayers;
		}
		void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			if (f & g_activeLayers)		f &= ~g_activeLayers;
			else							f |= g_activeLayers;
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};
	/* ------------------------------------------------------------------------ */ 
	/*  Invert points visibility state within the layer                         */ 
	/* ------------------------------------------------------------------------ */ 
	struct UnhideVisitor : public pcloud::Node::Visitor
	{
		bool visitNode(const pcloud::Node *n);

		inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			f |= g_activeLayers; 
		}
		void point(const pt::vector3d &pnt, uint index, ubyte &f)
		{
			f |= g_activeLayers; 
		}
	private:
		pcloud::Voxel *_currentVoxel;
	};
}