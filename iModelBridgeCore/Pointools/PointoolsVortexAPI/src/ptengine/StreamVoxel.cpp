
#include <ptengine/StreamVoxel.h>
#include <ptcloud2/Voxel.h>

#include <ptengine/StreamDataSource.h>


namespace pointsengine
{


StreamVoxel::StreamVoxel(void)
{
	clear();
}


StreamVoxel::StreamVoxel(StreamDataSource *initStreamDataSource, pcloud::Voxel *initVoxel)
{
	setStreamDataSource(initStreamDataSource);

	setVoxel(initVoxel);
}


void StreamVoxel::clear(void)
{
	setStreamDataSource(NULL);

	setVoxel(NULL);
}


bool StreamVoxel::operator < (const StreamVoxel &other) const
{
														// Sort based on last stream manager iteration (low to high)
	if(voxel != NULL && other.voxel != NULL)
	{
		return voxel->getLastStreamManagerIteration() < other.voxel->getLastStreamManagerIteration();
	}

	return false;
}


ptds::DataSource *StreamVoxel::getDataSource(void)
{
	if(getStreamDataSource())
	{
		return getStreamDataSource()->getDataSource();
	}

	return NULL;
}


} // End pointsengine namespace
