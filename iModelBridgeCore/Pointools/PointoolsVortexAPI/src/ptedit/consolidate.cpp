#include <ptedit/pointVisitors.h>

namespace ptedit
{
ConsolidateVisitor::ConsolidateVisitor( bool alllayers, bool flagState)
{
	_doSelState = flagState;
	_doAllLayers = alllayers;
}

bool ConsolidateVisitor::visitNode( const pcloud::Node *node )
{
	/* don't consolidate on refresh */ 
	if (g_editApplyMode == EditIntentFlagged) return false;

	pcloud::Node *n = const_cast<pcloud::Node*>(node);
	bool consolidateLayer = true;

	if (!_doAllLayers  && NodeCheck::isExcluded(n)) 
		return false;

	// looking for consistent point layer state
	// to enable freeing of the filter (layer) channel
	if (n->isLeaf())
	{
		/* need both layer and selection states */ 
		_lyrstate = 0;					
		_hassel = false;

		int i;
		for (i=0; i<EDT_MAX_THREADS; i++)	// 4 threads
		{
			_consolidateLyr[i] = true;	//reset
			_consolidateSel[i] = true;	//reset
			_pntLyrState[i] = 0;
		}
		bool conLyr = false;

		_currentVoxel = static_cast<pcloud::Voxel*>( n );

		pcloud::Voxel::LocalSpaceTransform lst;

		pcloud::DataChannel *filterChannel = _currentVoxel->channel(pcloud::PCloud_Filter);

		if (filterChannel)
		{
			if (!filterChannel->size())
			{
				_currentVoxel->destroyEditChannel();				//whats the point of this?
				_currentVoxel->flag( pcloud::PartHidden, false );
				_currentVoxel->flag( pcloud::PartSelected, false );
				return true;
			}

			/* added 30-05-09 */ 
			if (_currentVoxel->flag( pcloud::WholeSelected))
				_currentVoxel->flag( pcloud::WholeSelected, false);
	
			// cannot reliably consolidate a node that is not fully loaded
			bool isChannelComplete = _currentVoxel->numPointsEdited() < _currentVoxel->fullPointCount() * g_state.density
				? false : true;
				
			if (!_doSelState && !isChannelComplete) return false;

			boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
			_currentVoxel->channel(pcloud::PCloud_Filter)->getval(_lyrstate, 0);

			_selstate = _lyrstate & SELECTED_PNT_BIT ? true : false;
			_lyrstate &= ~SELECTED_PNT_BIT;

			VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
		
			if (_hassel)
			{
				_currentVoxel->flag( pcloud::PartSelected, true );
				if (!isChannelComplete) return false;
			}
	
			consolidateLayer = true;

			/* full consolidation, ie full occupancy of layer */ 
			for (i=0; i<EDT_MAX_THREADS; i++)
				if (!_consolidateLyr[i]) consolidateLayer = false;

			if (consolidateLayer)
			{
				conLyr = true;
				_currentVoxel->flag( pcloud::PartHidden, false );

				if (_currentVoxel->layers(1))
				{
					_currentVoxel->layers(0) = _lyrstate & ~SELECTED_PNT_BIT;
					_currentVoxel->layers(1) = 0;
					_currentVoxel->flag( pcloud::WholeHidden, _lyrstate & g_activeLayers ? false : true );
				}
			}
			/* update partial occupancy */ 
			for (i=1; i< EDT_MAX_THREADS; i++)
				_pntLyrState[0] |= _pntLyrState[i];

			_pntLyrState[0] &= ~SELECTED_PNT_BIT;
			//_currentVoxel->layers(1) = (_currentVoxel->layers(1) & _pntLyrState[0]);

			// check consolidation of selection state
			bool consolidateSel = true;			

			for (i=0; i< EDT_MAX_THREADS; i++)
				if (!_consolidateSel[i]) consolidateSel = false;

			if (conLyr && consolidateSel)
			{
				/* selection state */ 
				_currentVoxel->flag( pcloud::PartSelected, false );
				_currentVoxel->flag( pcloud::WholeSelected, _selstate );

				if (conLyr)	_currentVoxel->destroyEditChannel();	//only destroy channel if both layer and sel and consolidated
			}
		}
		else
		{
			_currentVoxel->flag( pcloud::PartHidden, false );
			_currentVoxel->flag( pcloud::PartSelected, false );
		}
	}
	return true;
}
} /* namespace ptedit */ 