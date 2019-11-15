/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"

#include <ptengine/VoxelLODRemoveVisitor.h>


namespace pointsengine
{

VoxelLODRemoveVisitor::VoxelLODRemoveVisitor(ClientID id)
{
	setClientID(id);
}


bool VoxelLODRemoveVisitor::voxel(pcloud::Voxel *voxel)
{
	if(voxel)
	{
		voxel->removeRequestLOD(getClientID());
		return true;
	}

	return false;
}

} // End pointsengine namespace

