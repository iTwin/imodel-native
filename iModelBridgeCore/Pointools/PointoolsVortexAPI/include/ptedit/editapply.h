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
		EditIntentFastest = 8,
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
			if ( !v->numPointsEdited() || (v->numPointsEdited() > v->lodPointCount()))
			{
				if (g_editWorkingMode & EditWorkOnAll)
					v->numPointsEdited( v->fullPointCount() );
				else
					v->numPointsEdited( v->lodPointCount() * g_state.density );
			}
		}		
		
		template < class Receiver, class Transformer >
		static void iteratePoints( pcloud::Voxel *v, Receiver &R, Transformer &T )
		{
			float am = g_editApplyMode & EditIncludeOOC ? 1.0f : MIN_FILTER;
			
			// check for override in working mode
			if (g_editWorkingMode == EditWorkOnAll)			am = 1.0f;
			if (g_editWorkingMode == EditWorkOnProportion)	am = g_state.density;

			int fp = v->numPointsEdited();

			{
				bool dumpAfterLoad = g_editApplyMode & EditIncludeOOC || !fp ? true : false;

				/* load points, dump if was non-empty */ 
				pointsengine::VoxelLoader load( v, am, false, false, dumpAfterLoad );

				if ( am > 0.999999f )
					v->numPointsEdited(0);

				if (g_editApplyMode & ( EditIntentRefresh | EditIntentFastest)) // CHECK, DIFFERS TO API
				{
					if (g_state.multithreaded)
						v->_iterateTransformedPoints4Threads( R, T, 0, v->numPointsEdited(), 0 );
					else
						v->_iterateTransformedPointsRange( R, T, 0, v->numPointsEdited(), 0 );
				}
				else
				{
					if (g_state.multithreaded)
						v->_iterateTransformedPoints4Threads( R, T, 0, 0, v->numPointsEdited() );
					else
						v->_iterateTransformedPointsRange( R, T, 0, 0, v->numPointsEdited() );
				}
			}
			if ( am > 0.999999f )	// out of core should load everything >
				v->numPointsEdited( v->fullPointCount() );  

		}
	};

	struct NodeCheck
	{
		inline static bool isExcluded( const pcloud::Node* n )
		{
			if (g_editApplyMode & EditIntentFlagged)
			{
				return ( n->isLeaf() && !n->flag(pcloud::Flagged) );
			}
			if (g_editApplyMode & EditIntentRefresh)
			{
				if (n->flag(pcloud::PartSelected) || n->layers(1)) 
					return false;
				return true;
			}
			return false;
		}
		inline static bool wholeInLayer( pcloud::Node* n, uint layer )
		{
			return n->layers(0) & layer;
		}
		inline static bool partInLayer( pcloud::Node* n, uint layer )
		{
			return n->layers(1) & layer;
		}	
	};
}