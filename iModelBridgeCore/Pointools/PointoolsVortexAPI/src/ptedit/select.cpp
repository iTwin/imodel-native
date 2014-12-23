#include <ptedit/pointVisitors.h>
#include <pt/project.h>

namespace ptedit {

//
// Clear Filter Visitor
//
bool DestroyFilterVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	if (n->isLeaf())
	{
		const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n))->destroyEditChannel();
	}	
	node->layers(1) = 0;
	return true;
}

//
// Clear Filter Visitor
//
bool ClearFilterVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	if (n->isLeaf())
	{
		const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n))->destroyEditChannel();
	}	
	node->flag(pcloud::PartSelected, false, false);
	node->flag(pcloud::WholeSelected, false, false);

	node->flag(pcloud::PartHidden, false, false);
	node->flag(pcloud::WholeHidden, false, false);

	node->flag(pcloud::Painted, false, false);
	
	node->layers(0) = 1;
	node->layers(1) = 0;
	return true;
}

//
// ClearFlagVisitor
//
ClearFlagVisitor::ClearFlagVisitor(pcloud::Flag fullflag, pcloud::Flag partflag, uint selval)
{
	_fflag = fullflag;
	_pflag = partflag;
	_selval = selval;
}

bool ClearFlagVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	if (node->flag(_fflag)) 
	{
		node->flag(_fflag, false, true);
		node->flag(_pflag, false, true);
		return false;
	}
	if (node->flag(_pflag))
	{
		node->flag(_pflag, false, false);
	
		if (n->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}
		}
	}
	return true;
}

/**
Visitor that removes points from active (ie visble and not locked) layers. 
*/
bool HidePointsVisitor::visitNode( const pcloud::Node *n )
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
		
	if ( !node->flag( pcloud::WholeSelected ) 
		&& !(node->flag( pcloud::PartSelected )))
		return false;
	
	if ( node->flag( pcloud::PartSelected ) || node->layers(1) & g_activeLayers )
	{
		node->layers(0) &= ~g_activeLayers;
		node->layers(1) |= g_activeLayers;

		node->flag( pcloud::WholeHidden, false );
		node->flag( pcloud::PartHidden, true );
		node->flag( pcloud::PartSelected, false );

		if (node->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}					
		}
	} 
	else if ( node->flag( pcloud::WholeSelected ) )
	{
		node->layers(0) &= ~g_activeLayers;
		node->layers(1) &= ~g_activeLayers;
		node->flag( pcloud::WholeHidden, true );
		node->flag( pcloud::WholeSelected, false );
	}
	return true;
}
//
//
//
bool DeselectAnyPointsVisitor::visitNode( const pcloud::Node *n )
{
	pcloud::Node *node = const_cast<pcloud::Node*>(n);
		
	if ( !node->flag( pcloud::WholeSelected ) && !(node->flag( pcloud::PartSelected )))
		return false;
	
	if ( node->flag( pcloud::PartSelected ) || node->layers(1) )
	{
		node->flag( pcloud::PartSelected, false );

		if (node->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);

			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}					
		}
	}
	else if ( node->flag( pcloud::WholeSelected ) )
	{
		node->flag( pcloud::WholeSelected, false );
	}
	return true;
} 

//
// DeselectPointsVisitor
//
bool DeselectPointsVisitor::visitNode( const pcloud::Node *n )
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
	
	if ( !node->flag( pcloud::WholeSelected ) && !(node->flag( pcloud::PartSelected )))
		return false;

	// Check against current isolation filter
	pt::BoundingBoxD bb = node->extents();

	// get extents in project space for isolation filter test
	pt::vector3d basePoint = pt::vector3d(pt::Project3D::project().registration().matrix()(3,0), 
		pt::Project3D::project().registration().matrix()(3,1), 
		pt::Project3D::project().registration().matrix()(3,2));
	bb.translateBy(-basePoint);

	SelectionResult selRes = intersectsIsolationFilter(bb);
	if (selRes == FullyOutside)
		return false;
	
	// If a node is fully in the active layer and fully inside the isolation filter then deselect	
	if ((node->layers(0) & g_activeLayers) && (selRes == FullyInside))
	{
		node->flag(pcloud::WholeSelected, false);
		node->flag(pcloud::PartSelected, false);
		if (node->isLeaf())
		{
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			_currentVoxel->destroyEditChannel();
		}
	}
	// If the node is part selected, partially in a layer or partially inside the isolation filter
	else if ( node->flag( pcloud::PartSelected ) 
		|| (node->layers(1) & g_activeLayers) 
		|| (node->layers(0) & g_activeLayers) 
		|| (selRes == PartiallyInside))
	{		
		if (node->isLeaf())
		{				
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			pcloud::Voxel::CoordinateSpaceTransform cst(const_cast<pcloud::PointCloud*>(_currentVoxel->pointCloud()), pt::ProjectSpace);

			// destroy the channel if it is no longer required
			if ((! (node->layers(1) & g_activeLayers)) && (selRes == FullyInside)) //no partial occupancy
			{
				_currentVoxel->destroyEditChannel();
				node->flag( pcloud::PartSelected, false );
			}
			else
			{				
				// If this node does not have an edit channel then it must be fully
				// in a layer but partially inside the isolation filter, create an edit channel
				// for it and initialize the edit channel to the layers that the node is fully in.
				if (!_currentVoxel->channel(pcloud::PCloud_Filter))					
				{
					ubyte fillValue = _currentVoxel->layers(0);
					if (node->flag( pcloud::WholeSelected ))
						fillValue |= SELECTED_PNT_BIT;
							
					_currentVoxel->buildEditChannel(fillValue);					
				}	

				if (_currentVoxel->channel(pcloud::PCloud_Filter))
				{
					boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
					VoxFiltering::iteratePoints( _currentVoxel, *this, cst );
					VoxFiltering::setPoint( _currentVoxel );
				}	

				node->flag(pcloud::WholeSelected, false);
				node->flag( pcloud::PartSelected, true );
			}
		}
	}
	
	
	return true;
}
//
// Select everything in layer
//
bool SelectPointsVisitor::visitNode( const pcloud::Node *n )
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
		
	if ( node->flag( pcloud::WholeSelected ) )	return false;
	
	if ( node->layers(1) & g_activeLayers ) 
	{
		node->flag( pcloud::PartSelected, true );
	
		if (node->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);

			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}					
		}		
	}
	else if ( node->layers(0) & g_activeLayers ) 
	{
		node->flag( pcloud::WholeSelected, true );
		if (node->isLeaf()) 
		{
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			_currentVoxel->destroyEditChannel();
		}
	}
	return true;
}
//
// DeselectHidden
//
bool DeselectHiddenVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
	
	if (node->flag(pcloud::WholeHidden))
		node->flag(pcloud::WholeSelected, false, true);

	if (n->isLeaf() && n->flag(pcloud::PartSelected)) 
	{
		pcloud::Voxel::LocalSpaceTransform lst;
		_currentVoxel = static_cast<pcloud::Voxel*>(node);
		
		boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
		VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
		VoxFiltering::setPoint( _currentVoxel );
	}
	return n->flag(pcloud::PartHidden);
}
//
// Invert Selection
//
bool InvertSelectionVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
	bool res = true;

	if (node->flag( pcloud::WholeHidden ))	return res;

	if ( !(node->layers(0) & g_activeLayers)
		&& !(node->layers(1) & g_activeLayers))
		return res;

	if (node->layers(0) & g_activeLayers)
	{
		if (!node->flag( pcloud::WholeSelected ) 
			&& !node->flag( pcloud::PartSelected ))
		{
			node->flag( pcloud::WholeSelected, true, true);
			res = false;
		}
		else if (node->flag( pcloud::WholeSelected )) 
		{
			node->flag( pcloud::WholeSelected, false, true);
			node->flag( pcloud::PartSelected, false, true );
			res = false;
		}
		if (!res) return res;
	}
	if ( node->flag( pcloud::PartSelected ) || node->layers(1) & g_activeLayers )
	{	
		node->flag( pcloud::PartSelected, true );

		if (n->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}
		}
	}
	return res;
}
//
// Isolate Visitor
//
bool IsolateSelectedVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	/* not per layer */ 
	if (node->layers(0) & g_activeLayers)
	{
		// completely unnselected
		if (!node->flag( pcloud::WholeSelected ) && !node->flag( pcloud::PartSelected ) )
		{
			// remove from layers
			node->layers(0) &= ~g_activeLayers;
			node->layers(1) &= ~g_activeLayers;
		}
		else if ( node->flag( pcloud::PartSelected ))
		{
			node->layers(0) &= ~g_activeLayers;
			node->layers(1) |= g_activeLayers;

			if (n->isLeaf())
			{
				pcloud::Voxel::LocalSpaceTransform lst;
				_currentVoxel = static_cast<pcloud::Voxel*>(node);
				if (_currentVoxel->channel(pcloud::PCloud_Filter))
				{
					boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
					VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
					VoxFiltering::setPoint( _currentVoxel );
				}
			}		
		}
		else // if whole selected nothing to do other than deselect
		{
			node->flag( pcloud::WholeSelected, false, true );		
			return false; 
		}
	}
	else if (node->layers(1) & g_activeLayers )
	{
		if (n->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}
		}		
	}
	return true;
}
//
// InvertVisibilityVisitor
//
bool InvertVisibilityVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	/* not per layer */ 
	if (node->layers(0) & g_activeLayers)
	{
		node->layers(0) &= ~g_activeLayers;
		node->flag( pcloud::WholeHidden, true, false );
	}
	else if (node->layers(1) & g_activeLayers)
	{
		if (n->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}
		}				
	}
	else 
	{
		node->layers(0) |= g_activeLayers;
		node->flag( pcloud::WholeHidden, false, false );
	}
	return true;
}
//
// Unhide Visitor
//
bool UnhideVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	/* not per layer */ 

	if (n->isLeaf())
	{
		pcloud::Voxel::LocalSpaceTransform lst;
		_currentVoxel = static_cast<pcloud::Voxel*>(node);

		if (n->layers(1) == g_activeLayers && !n->flag( pcloud::PartSelected ))
			_currentVoxel->destroyEditChannel();
		else if (_currentVoxel->channel(pcloud::PCloud_Filter))
		{
			boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
			VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
			VoxFiltering::setPoint( _currentVoxel );
		}
	}		
	node->flag( pcloud::WholeHidden, false );
	node->flag( pcloud::PartHidden, false );
	node->layers(0) |= g_activeLayers;
	node->layers(1) &= ~g_activeLayers;

	return true;
}

bool DeselectPointsInLayerVisitor::visitNode( const pcloud::Node *n )
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
	
	if ( !node->flag( pcloud::WholeSelected ) && !(node->flag( pcloud::PartSelected )))
		return false;

	// Check against current isolation filter
	pt::BoundingBoxD bb = node->extents();

	// get extents in project space for isolation filter test
	pt::vector3d basePoint = pt::vector3d(pt::Project3D::project().registration().matrix()(3,0), 
		pt::Project3D::project().registration().matrix()(3,1), 
		pt::Project3D::project().registration().matrix()(3,2));
	bb.translateBy(-basePoint);

	SelectionResult selRes = intersectsIsolationFilter(bb);
	if (selRes == FullyOutside)
		return false;
	
	// If a node is fully in the active layer and fully inside the isolation filter then deselect	
	if ((node->layers(0) & _targetLayer) && (selRes == FullyInside))
	{
		node->flag(pcloud::WholeSelected, false);
		node->flag(pcloud::PartSelected, false);
		if (node->isLeaf())
		{
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			_currentVoxel->destroyEditChannel();
		}
	}
	// If the node is part selected, partially in a layer or partially inside the isolation filter
	else if ( node->flag( pcloud::PartSelected ) 
		|| (node->layers(1) & _targetLayer) 
		|| (node->layers(0) & _targetLayer) 
		|| (selRes == PartiallyInside))
	{		
		if (node->isLeaf())
		{				
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			pcloud::Voxel::CoordinateSpaceTransform cst(const_cast<pcloud::PointCloud*>(_currentVoxel->pointCloud()), pt::ProjectSpace);

			// destroy the channel if it is no longer required - NOT in this case of targeted layer deselection
			// if node is not on target layer but part selected, this causes deselect
			if ((! (node->layers(1) & _targetLayer)) && (selRes == FullyInside)) //no partial occupancy or selection
			{
			//	_currentVoxel->destroyEditChannel();
			//	node->flag( pcloud::PartSelected, false );
			}
			else
			{				
				// If this node does not have an edit channel then it must be fully
				// in a layer but partially inside the isolation filter, create an edit channel
				// for it and initialize the edit channel to the layers that the node is fully in.
				if (!_currentVoxel->channel(pcloud::PCloud_Filter))					
				{
					ubyte fillValue = _currentVoxel->layers(0);
					if (node->flag( pcloud::WholeSelected ))
						fillValue |= SELECTED_PNT_BIT;
							
					_currentVoxel->buildEditChannel(fillValue);					
				}	

				if (_currentVoxel->channel(pcloud::PCloud_Filter))
				{
					boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
					VoxFiltering::iteratePoints( _currentVoxel, *this, cst );
					VoxFiltering::setPoint( _currentVoxel );
				}	

				node->flag(pcloud::WholeSelected, false);
				node->flag( pcloud::PartSelected, true );
			}
		}
	}
	return true;
}
//
// Select everything in layer
//
bool SelectPointsInLayerVisitor::visitNode( const pcloud::Node *n )
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
		
	if ( node->flag( pcloud::WholeSelected ) )	return false;
	
	if ( node->layers(1) & _targetLayer ) 
	{
		node->flag( pcloud::PartSelected, true );
	
		if (node->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);

			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
				boost::try_mutex::scoped_lock lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}					
		}		
	}
	else if ( node->layers(0) & _targetLayer ) 
	{
		node->flag( pcloud::WholeSelected, true );
		if (node->isLeaf()) 
		{
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			_currentVoxel->destroyEditChannel();
		}
	}
	return true;
}
//
// Compute Layer Bounds
//
ComputeLayerBoundsVisitor::ComputeLayerBoundsVisitor( ubyte layerMask, bool approx )
	: _layerMask( layerMask ), _approx(approx) 
{
	using namespace pt;

	_basepoint = vector3d(
		Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));
}
//
// compute a layers bounding box
//
bool ComputeLayerBoundsVisitor::visitNode(const pcloud::Node *n)
{
	if (n->layers(0) & _layerMask || n->layers(1) & _layerMask)
	{
		// compute down to leaf
		if (n->isLeaf())
		{
			pcloud::Node *node = const_cast<pcloud::Node*>(n);
			pcloud::Voxel *v = static_cast<pcloud::Voxel*>(node);
			
			if (v->channel(pcloud::PCloud_Filter) && !_approx)
			{
				boost::try_mutex::scoped_lock lock(v->mutex());

				pcloud::Voxel::CoordinateSpaceTransform pst(const_cast<pcloud::PointCloud*>(v->pointCloud()), pt::ProjectSpace);
				VoxFiltering::iteratePoints( v, *this, pst );
			}
			else
			{
				pt::BoundingBoxD bb( n->extents() );
				bb.translateBy( -_basepoint );

				_bounds[0].expandBy( bb );
			}
			return false;
		}
		else
		{
			return true;
		}
	}
	return false;
}
} /* namespace */ 