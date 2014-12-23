
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

