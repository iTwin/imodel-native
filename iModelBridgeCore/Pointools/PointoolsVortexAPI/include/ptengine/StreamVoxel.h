#pragma once

#include <ptds/DataSource.h>

namespace pcloud
{
	class Voxel;
}

namespace pointsengine
{

class StreamDataSource;

class StreamVoxel
{
protected:

	StreamDataSource		*	streamDataSource;
	pcloud::Voxel			*	voxel;

public:

								StreamVoxel				(void);
								StreamVoxel				(StreamDataSource *initStreamDataSource, pcloud::Voxel *initVoxel);

	void						clear					(void);

	void						setStreamDataSource		(StreamDataSource *initStreamDataSource)	{streamDataSource = initStreamDataSource;}
	StreamDataSource		*	getStreamDataSource		(void)										{return streamDataSource;}

	ptds::DataSource		*	getDataSource			(void);

	void						setVoxel				(pcloud::Voxel *initVoxel)					{voxel = initVoxel;}
	pcloud::Voxel			*	getVoxel				(void)										{return voxel;}

	bool						operator <				(const StreamVoxel &other) const;

};



} // End pointsengine namespace
