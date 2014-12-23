#pragma once

#include <ptcloud2/Voxel.h>
#include <PTRMI/DataBuffer.h>

using namespace pcloud;
using namespace ptds;

namespace pointsengine
{

class DataSourceVoxelSetCompare
{
public:

	bool operator()(const Voxel *v1, const Voxel *v2) const
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


class StreamManagerDataSource
{
public:

	typedef					std::multiset<Voxel *, DataSourceVoxelSetCompare>	VoxelSet;

protected:


	DataSourcePtr			dataSource;

	VoxelSet				voxelSet;

	DataSourceReadSet		readSet;
	PTRMI::DataBuffer		readSetBuffer;

protected:

	bool					verifyReadSet				(PTRMI::DataBuffer::Data *buffer, DataSourceReadSet *readSet);

public:

							StreamManagerDataSource		(DataSourcePtr initDataSource);
					   	   ~StreamManagerDataSource		(void);

	void					clear						(void);

	bool					beginReadSet				(void);
	bool					endReadSet					(void);

	DataSourceReadSet *		getReadSet					(void);

	void					setDataSource				(DataSourcePtr initDataSource);
	DataSourcePtr			getDataSource				(void);

	bool					addVoxel					(Voxel *voxel);
	bool					removeVoxel					(Voxel *voxel);
	unsigned int			getNumVoxels				(void);

	void					getVoxelIterators			(VoxelSet::iterator &start, VoxelSet::iterator &end);

	ptds::DataSize			executeReadSet				(void);

	ptds::DataSize			loadReadSetVoxelData		(void);
};



inline void StreamManagerDataSource::getVoxelIterators(VoxelSet::iterator &start, VoxelSet::iterator &end)
{
	start	= voxelSet.begin();
	end		= voxelSet.end();
}


inline unsigned int StreamManagerDataSource::getNumVoxels(void)
{
	return voxelSet.size();
}



} // End pointsengine namespace
