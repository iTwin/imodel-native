#pragma once

#include <ptcloud2/Voxel.h>

#include <ptengine/StreamVoxel.h>


namespace pointsengine
{

class VoxelPriorityQueueCompare
{
public:

	bool operator()(const pcloud::Voxel *v1, const pcloud::Voxel *v2) const
	{
		if(v1 && v2)
		{
															// Sort based on last stream manager iteration (low to high)
			return v1->getLastStreamManagerIteration() < v2->getLastStreamManagerIteration();
		}
															// Not valid, so return false
		return false;
	}

};

class VoxelPriorityQueue : public std::multiset<pcloud::Voxel *, VoxelPriorityQueueCompare>
{

};



class StreamVoxelPriorityQueue : public std::multiset<pointsengine::StreamVoxel>
{

};


} // End pointsengine namespace
