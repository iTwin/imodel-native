#pragma once
#include <ptcloud2/Node.h>
#include <ptengine/VoxelLODSet.h>
#include <ptcloud2/scene.h>
#include <ptengine/PointsVisitor.h>

namespace pointsengine
{

class VoxelLODRemoveVisitor : public PointsVisitor
{
public:

	typedef pointsengine::VoxelLODSet::ClientID	ClientID;

protected:

	ClientID		clientID;

public:

					VoxelLODRemoveVisitor	(ClientID id);

	void			setClientID				(ClientID id)					{clientID = id;}
	ClientID		getClientID				(void)							{return clientID;}

	bool			visitNode				(const pcloud::Node *n)			{return true;}
	bool			voxel					(pcloud::Voxel *voxel);
};

}
