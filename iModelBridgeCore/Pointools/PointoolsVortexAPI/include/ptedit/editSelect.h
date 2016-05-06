#pragma once

#include <pt/boundingbox.h>
#include <ptengine/pointsscene.h>
#include <ptengine/pointspager.h>
#include <ptengine/renderEngine.h>
#include <ptengine/engine.h>
#include <ptengine/clipManager.h>
#include <ptcloud2/voxel.h>
#include <ptgl/glviewstore.h>

#include <ptedit/edit.h>
#include <ptedit/editState.h>
#include <ptedit/editApply.h>
#include <ptedit/editNodeDef.h>
#include <ptedit/pointVisitors.h>
#include <ptedit/isolationFilter.h>

#include <pt/project.h>
#include <pt/timestamp.h>
#include <pt/datatree.h>
#include <pt/rect.h>
#include <omp.h>

#define	NO_TEST_POINT \
		inline static bool testPoint(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i) { return true; } 

#define NO_PAINT_POINT \
		static void paintPoint(int t, EditNodeDef *d, pcloud::Voxel *v, const pt::vector3d &p, uint i){}; 

#define TEST_POINT \
		static bool testPoint(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i)  \
		{ \
			if (!g_state.selPntTest) return true; \
			else return g_state.selPntTest(t,v,p,i); \
		}  

#define SELECTION_FILTER_DETAIL \
		static FilterType filterType() { return SelectionFilterType; }; \
		NO_PAINT_POINT \
		TEST_POINT 

#define PAINT_FILTER_DETAIL \
		static FilterType filterType() { return PaintFilterType; };\
		NO_TEST_POINT  

namespace ptedit
{
//---------------------------------------------------------------------------------------------
// Filter base class template
//---------------------------------------------------------------------------------------------
// used for most selection tools
//---------------------------------------------------------------------------------------------
template <typename PointFilter>
struct SelectionFilter
{
public:
	SelectionFilter(PointFilter &f) : 
		_filter(f), 
		_currentVoxel(NULL),		
		_isolationFilterIntersect(pointsengine::ClipManager::instance().getIntersectFunction()),
		_isolationFilterInside(pointsengine::ClipManager::instance().getInsideFunction()) 
		{	
			;
		}

	typedef PointFilter PointFilterType;

	//---------------------------------------------------------------------------------------------
	// for points export : but no longer used??? was not reliable
	//---------------------------------------------------------------------------------------------
	bool processVoxel(pcloud::Voxel* voxel, SelectionMode mode)
	{	
		_didSelect = 0;

		_basePoint = pt::vector3d(Project3D::project().registration().matrix()(3,0), 
				Project3D::project().registration().matrix()(3,1), 
				Project3D::project().registration().matrix()(3,2));

			pcloud::Voxel::CoordinateSpaceTransform cst(
				const_cast<pcloud::PointCloud*>(voxel->pointCloud()));

		processNode(voxel, cst);

		return _didSelect ? true : false;
	}
	//---------------------------------------------------------------------------------------------
	// process this filter through the point data 
	//---------------------------------------------------------------------------------------------
	bool processFilter()
	{
		_didSelect = 0;

		pt::TimeStamp t0, t1;
		t0.tick();

		_basePoint = pt::vector3d(pt::Project3D::project().registration().matrix()(3,0), 
				pt::Project3D::project().registration().matrix()(3,1), 
				pt::Project3D::project().registration().matrix()(3,2));

		int numScenes = pointsengine::thePointsScene().size();
		for (int sc=0; sc<numScenes; sc++)
		{
			pcloud::Scene* scene = pointsengine::thePointsScene()[sc];
			
			if (!g_state.inScope( scene )) continue;
			
			const pt::Bounds3DD &bounds = scene->projectBounds();
			int numClouds = scene->size();
			int cl;
			SelectionResult res = FullyOutside;
			
			// Test bounds against the current isolation filter 
			SelectionResult isolationRes = _isolationFilterIntersect(bounds.bounds());
			
			// Test bounds against the selection filter if not outside the isolation filter
			if (isolationRes != FullyOutside)
				res = PointFilter::intersect(_filter, bounds.bounds());

			// If the isolation filter returned PartiallyInside then the selection filter can only
			// be PartiallyInside and not FullyInside
			if ((isolationRes == PartiallyInside) && (res == FullyInside))
				res = PartiallyInside;
			
			if (res == FullyInside && !g_state.selPntTest)
			{
				for (cl=0;cl<numClouds;cl++)
				{
					pcloud::PointCloud *pc = scene->cloud(cl);
					
					if (!g_state.inScope(pc)) continue;
					
					pcloud::Node* root = const_cast<pcloud::Node*>(pc->root());

					if (root->layers(1) & g_activeLayers || g_state.selmode == UnhidePoint)
					{
						pcloud::Voxel::CoordinateSpaceTransform cst(pc);
						processNode( root, cst);
					}
					else if (root->layers(0) & g_activeLayers)
					{
						root->flag(pcloud::WholeSelected, g_state.selmode == SelectPoint ? true : false, true);
						root->flag(pcloud::PartSelected, false, true);
						++_didSelect;
					}
				}
			}
			else if (res != FullyOutside )
			{
				if (res == PartiallyInside || (g_state.selPntTest && res == FullyInside))
				{	
					for (cl=0;cl<numClouds;cl++)
					{
						pcloud::PointCloud *pc = scene->cloud(cl);

						if (!g_state.inScope( pc )) continue;

						pcloud::Voxel::CoordinateSpaceTransform cst(pc);

						processNode(const_cast<pcloud::Node*>(pc->root()), cst);
					}
				}
			}
		}

		t1.tick();
		_lastProcessTime = static_cast<int>(t1.delta_ms(t0, t1));
		return _didSelect ? true : false;
	}

	//---------------------------------------------------------------------------------------------
	// return the last processing time
	//---------------------------------------------------------------------------------------------
	int lastProcessTimeInMs() const { return _lastProcessTime; }
	
	//---------------------------------------------------------------------------------------------
	// per point processing
	//---------------------------------------------------------------------------------------------
	inline void point(const pt::vector3d &pnt, uint index, ubyte &f)	
	{
		static pcloud::Voxel *lvoxel=(pcloud::Voxel*)1;

		if (_isolationFilterInside(0, pnt)
			&& PointFilter::inside(0, _filter, pnt))
		{
			switch(PointFilter::filterType())
			{
			case SelectionFilterType:
				if (PointFilter::testPoint(0, _currentVoxel, pnt, index))
					selectPointInVoxel(f); 
				break;
			case PaintFilterType:
			
				if(f & g_activeLayers)
				{
					PointFilter::paintPoint(0, &_filter, _currentVoxel, pnt, index); 
					++_didSelect;
				}
				break;
			}
		}
	}

	//---------------------------------------------------------------------------------------------
	// multi-threaded version
	//---------------------------------------------------------------------------------------------
	inline void mt_point(int thread, const pt::vector3d &pnt, uint index, ubyte &f)	
	{
		
		if (_isolationFilterInside(thread, pnt)
			&& PointFilter::inside(thread, _filter, pnt))
		{
			switch(PointFilter::filterType())
			{
			case SelectionFilterType:
				if (PointFilter::testPoint(thread, _currentVoxel, pnt, index))
					selectPointInVoxel(f); 
				break;
			case PaintFilterType:
				if(f & g_activeLayers)
				{
					PointFilter::paintPoint(thread, &_filter, _currentVoxel, pnt, index);
					++_didSelect;
				}
				break;
			}
		}
	}
	//---------------------------------------------------------------------------------------------
	// returns num points processed in last operation
	//---------------------------------------------------------------------------------------------
	int didSelect() const { return _didSelect; }

protected:
	int				_lastProcessTime;
	pt::vector3d	_basePoint;
	pcloud::Voxel	*_currentVoxel;
	int				_didSelect;
	PointFilter		_filter;

	// Functions for isolation filter methods
	IsolationFilter::IntersectCallback	_isolationFilterIntersect;
	IsolationFilter::InsideCallback		_isolationFilterInside;

	//---------------------------------------------------------------------------------------------
	// do the point level selection
	//---------------------------------------------------------------------------------------------
	inline void selectPointInVoxel(ubyte &f)
	{	
		switch (g_state.selmode)
		{
		case SelectPoint:
			if (f & g_activeLayers)
			{
				f |= SELECTED_PNT_BIT;
				++_didSelect;
			}
			break;

		case DeselectPoint:
			if (f & g_activeLayers)
			{
				f &= ~SELECTED_PNT_BIT;
				++_didSelect;
			}
			break;

		case UnhidePoint:
			if (!(f & g_activeLayers))	
			{
				f |= g_activeLayers;
				++_didSelect;
			}
			break;
		}
	}

private:
	std::vector<pcloud::Voxel *> _voxellist;


	//---------------------------------------------------------------------------------------------
	//process the Node for the selection / unhide / paint operation
	//---------------------------------------------------------------------------------------------
	int processNode(const pcloud::Node* node, pcloud::Voxel::CoordinateSpaceTransform &cst)
	{
		if (NodeCheck::isExcluded(node)) return 0;											// check for exclusion by current processing setup
		
		int ret = 0;
		pt::BoundingBoxD box = node->extents();												// move node extents into current project space
		box.translateBy(-_basePoint);
																							
		SelectionResult isolationRes = _isolationFilterIntersect(box);						// check intersection with the current isolation filter and return if completely outside
		if (isolationRes == FullyOutside)
			return 0;

		SelectionResult res = PointFilter::intersect(_filter, box);							// check intersection with selection volume or method,														
		if ((isolationRes == PartiallyInside) && (res == FullyInside))						// if the isolation filter returned PartiallyInside then the selection can only be PartiallyInside as well
			res = PartiallyInside;

		pcloud::Node *n = const_cast<pcloud::Node*>(node);

		if ( ! res	||																		// no intersection so nothing else to do
			!(node->layers(0) & g_activeLayers) 
				&& !(node->layers(1) & g_activeLayers))										// anything to do in active layers?
			return 0;

		// deal with unhide mode seperately
		if (PointFilter::filterType() ==  SelectionFilterType 
			&& g_state.selmode == UnhidePoint)
		{
			return processUnhidePointOp( n, res, cst );
		}

		// selection and paint modes
		if (res == FullyInside										// node is fully included in selection test
			&& n->layers(0) & g_activeLayers						// in layer
			&& PointFilter::filterType() ==  SelectionFilterType	// doing selection
			&& !g_state.selPntTest									// no per point test (ie contraints)
			)
		{
			ret = processSelectionFullInclusion( n );
		}
		else					
		{
			ret = processSelectionPartialInclusion( n, cst );
		}
		return ret;
	}
	//---------------------------------------------------------------------------------------------
	// node is only partially overlapped / intersected by selection - may require per point check
	//---------------------------------------------------------------------------------------------
	int processSelectionPartialInclusion( pcloud::Node *n, pcloud::Voxel::CoordinateSpaceTransform &cst)
	{
		int ret = 0;

		if (n->isLeaf())
		{
			_currentVoxel = static_cast<pcloud::Voxel*>(n);									// nasty, but safe

			if (  g_state.selmode == DeselectPoint
				&& !(_currentVoxel->flag(pcloud::WholeSelected) 
				|| _currentVoxel->flag(pcloud::PartSelected)))								// nothing selected in this node to deselect so can exit
				return 0;
			
			ubyte initial_val = _currentVoxel->layers(0);									// initial value for per point filter channel 

			/* do this if not painting */ 
			if (PointFilter::filterType() != PaintFilterType )
			{
				if (g_state.selmode == SelectPoint)
				{
					if (!_currentVoxel->flag(pcloud::WholeSelected))						// nothing to do if it is already enitrely selected
					{
						_currentVoxel->buildEditChannel( initial_val );					// may do nothing if already exists, harmless
					}
					else return 0;
				}
				if (g_state.selmode == DeselectPoint)
				{
					if ( _currentVoxel->flag(pcloud::WholeSelected) )						// if whole selected this has to change	for deselection	
					{
						initial_val |= SELECTED_PNT_BIT;											
						_currentVoxel->flag( pcloud::WholeSelected, false );				// remove whole selected flag
						_currentVoxel->flag( pcloud::PartSelected, true );					// add partial selected flag
						_currentVoxel->buildEditChannel( initial_val );					// and start with selected point value
					}
				}
			}

            std::lock_guard<std::mutex> lock(_currentVoxel->mutex());                   // need a lock here, will block

			int selected = _didSelect;
			VoxFiltering::iteratePoints( _currentVoxel, *this, cst );						// run selection test per point
			VoxFiltering::setPoint( _currentVoxel );	

			if (selected < _didSelect)
			{
				ret = 2;	// partial inclusion, ie per point did actually select
			}
			else if (!_currentVoxel->flag( pcloud::PartSelected ) && !_currentVoxel->layers(1) )	//if nothing selected or no partial layers
			{		
				_currentVoxel->destroyEditChannel();												// delete filter channel
			}
		}
		else
		{
			/* child  */ 
			const pcloud::Node * child=0;

			for (int oct=0; oct<8;oct++)
			{
				child = n->child(oct);
				if (child)
				{
					/* no need to iterate if already entirely selected */ 
					if (child->isLeaf() && 
						(
						(g_state.selmode == SelectPoint && n->flag(pcloud::WholeSelected))
						|| 
						(g_state.selmode == DeselectPoint && !(n->flag(pcloud::WholeSelected) || n->flag(pcloud::PartSelected)))
						))
					{
						return 0;
					}
					ret |= processNode(child, cst);
				}
			}
		}
		/* set flags */ 
		if (ret)
		{
			switch(PointFilter::filterType())
			{ 
			case SelectionFilterType:
		
				n->flag( pcloud::WholeSelected, false );
				n->flag( pcloud::PartSelected, true );
				break;

			case PaintFilterType:
				n->flag( pcloud::Painted, true );
			}
		}
		return ret;
	}
	//---------------------------------------------------------------------------------------------
	// process this node entirely as it is fully included in operation
	//---------------------------------------------------------------------------------------------
	int processSelectionFullInclusion( pcloud::Node* n )
	{
        _didSelect += static_cast<int>(n->lodPointCount());

		switch (g_state.selmode)
		{
		case SelectPoint:
			n->flag(pcloud::WholeSelected, true, true);
			n->flag(pcloud::PartSelected, false, true);
			break;

		case DeselectPoint:
			n->flag(pcloud::WholeSelected, false, true);
			n->flag(pcloud::PartSelected, false, true);
			break;
		}
		
		if (!n->layers(1) && !n->flag( pcloud::PartSelected))	//if no partial layer occupancy or selection
		{		
			n->traverseTopDown( &s_destroyFilterVisitor );		// delete partial occupancy / selection data now not needed for this node or child nodes
		}
		else if (g_state.selmode == SelectPoint)				// otherwise need per point select
		{
			SelectPointsVisitor v;
			n->traverseTopDown( &v );
		}
		else													// or delselect
		{
			DeselectPointsVisitor v;
			n->traverseTopDown(&v);
		}
		return 1;
	}
	//---------------------------------------------------------------------------------------------
	// do unhide operation on node, returns processed
	//---------------------------------------------------------------------------------------------
	int processUnhidePointOp( pcloud::Node* n, SelectionResult res, pcloud::Voxel::CoordinateSpaceTransform &cst)
	{
		if (res == PartiallyInside || n->layers(1) & g_activeLayers)
		{
			if (n->isLeaf())
			{
				int selected = _didSelect;
				_currentVoxel = static_cast<pcloud::Voxel*>(const_cast<pcloud::Node*>(n));

				ubyte initial_val = 0;
				if ( _currentVoxel->flag(pcloud::WholeSelected) ) initial_val |= SELECTED_PNT_BIT;
				
                std::lock_guard<std::mutex> lock(_currentVoxel->mutex());

				/* if currently fully selected, no filter channel will exist. We need to fill a channel with sel value */ 
				_currentVoxel->buildEditChannel( initial_val );

				/* load a minimal number of points if nothing is loaded to ensure we set a filter point */ 
				VoxFiltering::iteratePoints( _currentVoxel, *this, cst );
				VoxFiltering::setPoint( _currentVoxel );

				return selected < _didSelect ? 1 : 0;
			}
			else
			{
				const pcloud::Node * child=0;

				int ret = 0;

				for (int oct=0; oct<8;oct++)
				{
					child = n->child(oct);
					if (child)
					{
						ret |= (uint)processNode(child, cst);
					}
				}
				return ret;
			}
		}
		else
		{
			UnhideVisitor v;
			n->traverseTopDown(&v);
			return 1;
		}
	}
};
}