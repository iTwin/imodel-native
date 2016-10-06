#include "PointoolsVortexAPIInternal.h"

#include <ptengine/VoxelQueryStateRemoveVisitor.h>

namespace pointsengine
{

VoxelQueryStateRemoveVisitor::VoxelQueryStateRemoveVisitor(ClientID id)
{
    clientID = id;
}

bool VoxelQueryStateRemoveVisitor::voxel(pcloud::Voxel * voxel)
{
    if (voxel)
    {
        VoxelQueryStateSet &stateSet = voxel->getVoxelQueryStateSet();

        stateSet.removeQueryState(getClientID());

        return true;
    }
    
    return false;
}

}
