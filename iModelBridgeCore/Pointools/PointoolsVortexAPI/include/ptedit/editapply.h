#pragma once
#include <pt/boundingbox.h>
#include <ptengine/pointsscene.h>
#include <ptengine/pointspager.h>
#include <ptengine/renderEngine.h>
#include <ptengine/engine.h>
#include <ptcloud2/voxel.h>

namespace ptedit
{
	enum EditApplyMode
	{
		EditNormal = 0,
		EditIncludeOOC = 1,
		EditIntentRegen = 2,
		EditIntentRefresh = 4,
		EditViewBased = 16,
		EditIntentFlagged = 32
	};
	enum EditWorkingMode
	{
		EditWorkOnAll = 1,
		EditWorkOnView = 2,
		EditWorkOnProportion = 3
	};
	extern uint g_editApplyMode;
	extern uint g_editWorkingMode;

	struct VoxFiltering
	{
		static void setPoint( pcloud::Voxel *v )	// sets the number of points editing to ensure correct rendering
		{
			if ( !(g_editApplyMode & EditIntentRefresh) 
				&& (!v->numPointsEdited() || (v->numPointsEdited() > v->lodPointCount())))
			{
				if ( g_editWorkingMode & EditWorkOnAll || (g_editApplyMode & EditIncludeOOC) )
					v->numPointsEdited( v->fullPointCount() );
				else
					v->numPointsEdited( v->lodPointCount() * g_state.density );
			}
		}		
		
		// iterations should of one of 3 types:
		//	1. Operational, ie. new op on stack. 
		//		In this case the numPointsEdited is the limit. If this is 0 we process all points in the current LOD
		//	2. Full Regeneration
		//		numPointsEdited gets set to zero to force full evaluation
		//  3. Refresh
		//		we only want to process points that are not processed already, these are numPointsEdited until the end of the LOD
		//		in this case the numPointsEdited must not be set until the stack is completely executed.

		template < class Receiver, class Transformer >
		static void iteratePoints( pcloud::Voxel *v, Receiver &R, Transformer &T )
		{
			// the amount to load is either everything or just the minimum for processing if there is no data
			float am = g_editApplyMode & EditIncludeOOC ? 1.0f : MIN_FILTER;
			
			// check for override in working mode
			if (g_editWorkingMode == EditWorkOnAll)			am = 1.0f;
			if (g_editWorkingMode == EditWorkOnProportion)	am = g_state.density;

			// number of points already processed
			int fp = v->numPointsEdited();

			{
				// for ooc we will dump data after loading + processing
				bool dumpAfterLoad = (g_editApplyMode & EditIncludeOOC || !fp) && am > MIN_FILTER ? true : false;

				/* load points, dump if was non-empty */ 
				pointsengine::VoxelLoader load( v, am, false, false, dumpAfterLoad );

				// if amount = 1.0f, this means points are being loaded, so reset number edit to 0 for full processing
				if ( am > 0.999999f )	// in the case of a EditModeRefresh, this voxel would be skipped anyway
					v->numPointsEdited(0);

				if (g_editApplyMode & EditIntentRefresh ) 
				{
					if (g_state.multithreaded)
						v->_iterateTransformedPoints4Threads( R, T, 0, 0/*v->numPointsEdited()*/, 0 ); // this is a hack because can't get optimised version to work
					else
						v->_iterateTransformedPointsRange( R, T, 0, 0/*v->numPointsEdited()*/, 0 );
				}
				else
				{
					if (g_state.multithreaded)
						v->_iterateTransformedPoints4Threads( R, T, 0, 0, v->numPointsEdited() );
					else
						v->_iterateTransformedPointsRange( R, T, 0, 0, v->numPointsEdited() );

					if ( am > 0.999999f )	// numbers must match exactly
						v->numPointsEdited( v->fullPointCount() ); 
				}
			}
		}
	};

	struct NodeCheck
	{
		inline static bool isExcluded( const pcloud::Node* n )
		{
			if (g_editApplyMode & EditIntentFlagged)
			{
				if ( !n->flag(pcloud::Flagged) ) 
					return true;
			}
			if (g_editApplyMode & EditIntentRefresh)
			{
				if (n->isLeaf())
				{
					const pcloud::Voxel*v = static_cast<const pcloud::Voxel*>(n);
					
					if (v->numPointsEdited() == v->fullPointCount())
						return true;	// all points already processed
					if (!(g_editApplyMode & EditIncludeOOC) && v->numPointsEdited() >= v->lodPointCount())
						return true;
				}	
				return false;
			}
			return false;
		}
		inline static bool wholeInLayer( pcloud::Node* n, uint layer )
		{
			return (n->layers(0) & layer) != 0;
		}
		inline static bool partInLayer( pcloud::Node* n, uint layer )
		{
			return (n->layers(1) & layer) != 0;
		}	
	};
}