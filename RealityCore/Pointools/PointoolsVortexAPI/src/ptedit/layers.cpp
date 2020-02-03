/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"
#include <ptedit/editNodeDef.h>
#include <ptedit/pointvisitors.h>
namespace ptedit
{
//
// Copy to Layer
//
bool CopyToLayerVisitor::visitNode( const pcloud::Node *n )
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);
		
	if ( !node->flag( pcloud::WholeSelected ) && !(node->flag( pcloud::PartSelected )) )
		return false;

	node->flag( pcloud::WholeHidden, false );

	if ( node->flag( pcloud::PartSelected ) )
	{
		node->layers(1) |= g_currentLayer;
		node->flag( pcloud::WholeHidden, false );
		node->flag( pcloud::PartHidden, true );

		if (node->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
                std::lock_guard<std::mutex> lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}					
		}
	}
	else 
	{
		node->layers(0) |= g_currentLayer;
	}

	return true;
}	
//
// Move to Layer
//
bool MoveToLayerVisitor::visitNode( const pcloud::Node *n )
{
	if (NodeCheck::isExcluded(n)) return false;
	pcloud::Node *node = const_cast<pcloud::Node*>(n);
				
	//if ( !node->flag( pcloud::WholeSelected ) && !(node->flag( pcloud::PartSelected )) )
	//	return false;

	if ( node->flag( pcloud::PartSelected ) )
	{
		node->layers(1) |= node->layers(0) | g_currentLayer;
		node->layers(0) = 0;

		node->flag( pcloud::WholeHidden, false );
		node->flag( pcloud::PartHidden, true );

		if (node->isLeaf())
		{
			pcloud::Voxel::LocalSpaceTransform lst;
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			if (_currentVoxel->channel(pcloud::PCloud_Filter))
			{
                std::lock_guard<std::mutex> lock(_currentVoxel->mutex());
				VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
				VoxFiltering::setPoint( _currentVoxel );
			}					
		}
	}
	else if ( node->flag( pcloud::WholeSelected ) )
	{
		node->layers(0) = g_currentLayer;
		node->layers(1) = 0;

		node->flag( pcloud::WholeHidden, false, true );
		node->flag( pcloud::PartHidden, false, true );

		if (node->isLeaf())
		{
			_currentVoxel = static_cast<pcloud::Voxel*>(node);
			_currentVoxel->destroyEditChannel();
		}
	}

	return true;
}
//
// update layer visitor
//
bool UpdateLayerVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	if ( NodeCheck::wholeInLayer( node, g_visibleLayers ) )
	{
		node->flag( pcloud::WholeHidden, false, false );
		node->flag( pcloud::PartHidden, false, false );
	}
	else if ( NodeCheck::partInLayer( node, g_visibleLayers ) )
	{
		node->flag(pcloud::WholeHidden, false, true);
		node->flag(pcloud::PartHidden, true, false);
	}
	else
	{
		node->flag(pcloud::WholeHidden, true, true);
		node->flag(pcloud::PartHidden, false, false);				
	}
	return true;
}
//
// Set Hidden Layer Visitor
//
SetHiddenLayerVisitor::SetHiddenLayerVisitor()
{
	g_activeLayers = 0;
	g_currentLayer = 0;
	g_visibleLayers = 0;
}
bool SetHiddenLayerVisitor::visitNode(const pcloud::Node *n)
{
	if (NodeCheck::isExcluded(n)) return false;

	pcloud::Node *node = const_cast<pcloud::Node*>(n);

	if ( !(node->layers(0) | node->layers(1) ) )
	{
		node->flag( pcloud::WholeHidden, false, false );
		node->flag( pcloud::PartHidden, false, false );
	}
	else if (node->layers(1))
	{
		node->flag( pcloud::WholeHidden, false, false );
		node->flag( pcloud::PartHidden, true, false );
	}
	else if ( !(node->layers(1)) )
	{
		node->flag( pcloud::WholeHidden, true, false );
		node->flag( pcloud::PartHidden, false, false );
	}
	return true;
}
//
} /* namespace ptedit */ 
